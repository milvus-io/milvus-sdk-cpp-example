// Licensed to the LF AI & Data foundation under one
// or more contributor license agreements. See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership. The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <random>
#include <string>
#include <thread>

#include "milvus/MilvusClient.h"

namespace util {
void
CheckStatus(std::string&& prefix, const milvus::Status& status) {
    if (!status.IsOk()) {
        std::cout << "Failed to " << prefix << ", error: " << status.Message() << std::endl;
        exit(1);
    } else {
        std::cout << "Succeed to " << prefix << std::endl;
    }
}

std::vector<float>
GenerateFloatVector(int dimension) {
    std::random_device rd;
    std::mt19937 ran(rd());
    std::uniform_real_distribution<float> float_gen(0.0, 1.0);
    std::vector<float> vector(dimension);
    for (auto d = 0; d < dimension; ++d) {
        vector[d] = float_gen(ran);
    }
    return std::move(vector);
}
}

int
main(int argc, char* argv[]) {
    printf("Example start...\n");

    auto client = milvus::MilvusClient::Create();

    milvus::ConnectParam connect_param{"localhost", 19530, "root", "Milvus"};
    auto status = client->Connect(connect_param);
    util::CheckStatus("connect to milvus server", status);

    // print the server version
    std::string version;
    status = client->GetVersion(version);
    util::CheckStatus("get server version", status);
    std::cout << "The milvus server version is: " << version << std::endl;

    // drop the collection if it exists
    const std::string collection_name = "MY_PROGRAM_COLLECTION";
    status = client->DropCollection(collection_name);

    // create a collection
    const std::string field_id = "user_id";
    const std::string field_name = "user_name";
    const std::string field_age = "user_age";
    const std::string field_face = "user_face";
    const uint32_t dimension = 128;

    // collection schema, create collection
    milvus::CollectionSchema collection_schema(collection_name);
    collection_schema.AddField({field_id, milvus::DataType::INT64, "user id", true, false});
    milvus::FieldSchema varchar_scheam{field_name, milvus::DataType::VARCHAR, "user name"};
    varchar_scheam.SetMaxLength(100);
    collection_schema.AddField(varchar_scheam);
    collection_schema.AddField({field_age, milvus::DataType::INT8, "user age"});
    collection_schema.AddField(
        milvus::FieldSchema(field_face, milvus::DataType::FLOAT_VECTOR, "face signature").WithDimension(dimension));

    status = client->CreateCollection(collection_schema);
    util::CheckStatus("create collection " + collection_name, status);

    // create index (required after 2.2.0)
    milvus::IndexDesc index_vector(field_face, "", milvus::IndexType::IVF_FLAT, milvus::MetricType::COSINE);
    index_vector.AddExtraParam(milvus::NLIST, "100");
    status = client->CreateIndex(collection_name, index_vector);
    util::CheckStatus("create index on vector field", status);

    milvus::IndexDesc index_varchar(field_name, "", milvus::IndexType::TRIE);
    status = client->CreateIndex(collection_name, index_varchar);
    util::CheckStatus("create index on varchar field", status);

    milvus::IndexDesc index_sort(field_age, "", milvus::IndexType::STL_SORT);
    status = client->CreateIndex(collection_name, index_sort);
    util::CheckStatus("create index on integer field", status);

    // tell server prepare to load collection
    status = client->LoadCollection(collection_name);
    util::CheckStatus("load collection " + collection_name, status);

    // insert some rows
    const int64_t row_count = 1000;
    std::vector<int64_t> insert_ids;
    std::vector<std::string> insert_names;
    std::vector<int8_t> insert_ages;
    std::vector<std::vector<float>> insert_vectors;
    for (auto i = 0; i < row_count; ++i) {
        insert_ids.push_back(i);
        insert_names.push_back("user_" + std::to_string(i));
        insert_ages.push_back(static_cast<int8_t>(i%100));
        insert_vectors.emplace_back(util::GenerateFloatVector(dimension));
    }

    std::vector<milvus::FieldDataPtr> fields_data{
        std::make_shared<milvus::Int64FieldData>(field_id, insert_ids),
        std::make_shared<milvus::VarCharFieldData>(field_name, insert_names),
        std::make_shared<milvus::Int8FieldData>(field_age, insert_ages),
        std::make_shared<milvus::FloatVecFieldData>(field_face, insert_vectors)};
    milvus::DmlResults dml_results;
    status = client->Insert(collection_name, "", fields_data, dml_results);
    util::CheckStatus("insert", status);
    std::cout << "Successfully insert " << dml_results.IdArray().IntIDArray().size() << " rows." << std::endl;

    {
        // verify the row count of the partition is 999 by query(count(*))
        // set to STRONG level to ensure the delete request is done by server
        milvus::QueryArguments q_count{};
        q_count.SetCollectionName(collection_name);
        q_count.AddOutputField("count(*)");
        q_count.SetConsistencyLevel(milvus::ConsistencyLevel::STRONG);

        milvus::QueryResults count_resutl{};
        status = client->Query(q_count, count_resutl);
        util::CheckStatus("query count(*)", status);
        std::cout << "partition count(*) = " << count_resutl.GetRowCount() << std::endl;
    }

    {
        // query
        milvus::QueryArguments q_arguments{};
        q_arguments.SetCollectionName(collection_name);
        q_arguments.SetFilter(field_id + " in [5, 10]");
        q_arguments.AddOutputField(field_id);
        q_arguments.AddOutputField(field_name);
        q_arguments.AddOutputField(field_age);
        // set to EVENTUALLY level since the last query uses STRONG level and no data changed
        q_arguments.SetConsistencyLevel(milvus::ConsistencyLevel::EVENTUALLY);

        std::cout << "Query with expression: " << q_arguments.Filter() << std::endl;
        milvus::QueryResults query_resutls{};
        status = client->Query(q_arguments, query_resutls);
        util::CheckStatus("query", status);

        milvus::EntityRows rows;
        status = query_resutls.OutputRows(rows);
        util::CheckStatus("get output rows", status);
        for (auto& row : rows) {
            std::cout << "\t" << row.dump() << std::endl;
        }
    }

    {
        // do search
        // this collection has only one vector field, no need to set the AnnsField name
        milvus::SearchArguments s_arguments{};
        s_arguments.SetCollectionName(collection_name);
        s_arguments.SetLimit(10);
        s_arguments.AddOutputField(field_name);
        s_arguments.AddOutputField(field_age);
        std::string filter_expr = field_age + " > 40";
        s_arguments.SetFilter(filter_expr);
        // set to BOUNDED level to accept data inconsistence within a time window(default is 5 seconds)
        s_arguments.SetConsistencyLevel(milvus::ConsistencyLevel::BOUNDED);

        auto q_number_1 = 5;
        auto q_number_2 = 10;
        s_arguments.AddFloatVector(field_face, insert_vectors[q_number_1]);
        s_arguments.AddFloatVector(field_face, insert_vectors[q_number_2]);
        std::cout << "Searching the No." << q_number_1 << " and No." << q_number_2
                  << " with expression: " << filter_expr << std::endl;

        milvus::SearchResults search_results{};
        status = client->Search(s_arguments, search_results);
        util::CheckStatus("search", status);

        for (auto& result : search_results.Results()) {
            auto& ids = result.Ids().IntIDArray();
            auto& distances = result.Scores();
            if (ids.size() != distances.size()) {
                std::cout << "Illegal result!" << std::endl;
                continue;
            }

            std::cout << "Result of one target vector:" << std::endl;

            milvus::EntityRows rows;
            status = result.OutputRows(rows);
            util::CheckStatus("get output rows", status);
            for (auto& row : rows) {
                std::cout << "\t" << row.dump() << std::endl;
            }
        }
    }

    // release collection
    status = client->ReleaseCollection(collection_name);
    util::CheckStatus("release collection " + collection_name, status);

    // drop index
    status = client->DropIndex(collection_name, field_face);
    util::CheckStatus("drop index for field " + field_face, status);

    // drop collection
    status = client->DropCollection(collection_name);
    util::CheckStatus("drop collection " + collection_name, status);

    status = client->Disconnect();
    util::CheckStatus("disconnect to milvus server", status);
    return 0;
}
