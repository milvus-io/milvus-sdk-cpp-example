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

#include "milvus/MilvusClientV2.h"

namespace util {
void
CheckStatus(std::string&& msg, const milvus::Status& status) {
    if (!status.IsOk()) {
        std::cout << "Failed to " << msg << ", error: " << status.Message() << std::endl;
        exit(1);
    } else {
        std::cout << "Succeed to " << msg << std::endl;
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

    auto client = milvus::MilvusClientV2::Create();

    // connect
    milvus::ConnectParam connect_param{"localhost", 19530, "root", "Milvus"};
    auto status = client->Connect(connect_param);
    util::CheckStatus("connect milvus server", status);

    // check server healthy
    milvus::CheckHealthResponse resp_health;
    status = client->CheckHealth(milvus::CheckHealthRequest(), resp_health);
    util::CheckStatus("check milvus server healthy", status);
    if (resp_health.IsHealthy()) {
        std::cout << "The milvus server is healthy" << std::endl;
    } else {
        std::cout << "The milvus server is unhealthy, reasons: " << std::endl;
        for (auto reason : resp_health.Reasons()) {
            std::cout <<  reason << std::endl;
        }
            
        for (auto quota : resp_health.QuotaStates()) {
            std::cout <<  quota << std::endl;
        }
    }

    // print the server version
    std::string version;
    status = client->GetServerVersion(version);
    util::CheckStatus("get server version", status);
    std::cout << "The milvus server version is: " << version << std::endl;

    // print the SDK version
    client->GetSDKVersion(version);
    std::cout << "The CPP SDK version is: " << version << std::endl;

    // drop the collection if it exists
    const std::string collection_name = "MY_PROGRAM_COLLECTION";
    status = client->DropCollection(milvus::DropCollectionRequest().WithCollectionName(collection_name));

    // names
    const std::string field_id = "user_id";
    const std::string field_name = "user_name";
    const std::string field_age = "user_age";
    const std::string field_embedding = "user_face";
    const uint32_t dimension = 128;

    // collection schema, create collection
    milvus::CollectionSchema collection_schema(collection_name);
    collection_schema.AddField({field_id, milvus::DataType::INT64, "user id", true, false});
    collection_schema.AddField(
        milvus::FieldSchema(field_name, milvus::DataType::VARCHAR, "user name").WithMaxLength(100)
    );
    collection_schema.AddField({field_age, milvus::DataType::INT8, "user age"});
    collection_schema.AddField(
        milvus::FieldSchema(field_embedding, milvus::DataType::FLOAT_VECTOR, "face signature").WithDimension(dimension));

    status = client->CreateCollection(milvus::CreateCollectionRequest()
                                          .WithCollectionSchema(std::make_shared<milvus::CollectionSchema>(std::move(collection_schema)))
                                          .WithConsistencyLevel(milvus::ConsistencyLevel::BOUNDED));
    util::CheckStatus("create collection " + collection_name, status);

    // create index
    milvus::IndexDesc index_vector(field_embedding, "", milvus::IndexType::IVF_FLAT, milvus::MetricType::COSINE);
    index_vector.AddExtraParam(milvus::NLIST, "100");
    milvus::IndexDesc index_varchar(field_name, "", milvus::IndexType::TRIE);
    milvus::IndexDesc index_sort(field_age, "", milvus::IndexType::STL_SORT);
    status = client->CreateIndex(milvus::CreateIndexRequest()
                                     .WithCollectionName(collection_name)
                                     .AddIndex(std::move(index_vector))
                                     .AddIndex(std::move(index_varchar))
                                     .AddIndex(std::move(index_sort)));
    util::CheckStatus("create index on integer field", status);

    // tell server prepare to load collection
    status = client->LoadCollection(milvus::LoadCollectionRequest()
                                        .WithCollectionName(collection_name)
                                        .WithReplicaNum(1));
    util::CheckStatus("load collection " + collection_name, status);

    // insert some rows
    const int64_t row_count = 1000;
    milvus::EntityRows rows;
    for (auto i = 0; i < row_count; ++i) {
        milvus::EntityRow row;
        row[field_id] = i;
        row[field_name] = "user_" + std::to_string(i);
        row[field_age] = i % 100;
        row[field_embedding] = util::GenerateFloatVector(dimension);
        rows.emplace_back(std::move(row));
    }

    milvus::InsertResponse resp_insert;
    status = client->Insert(
        milvus::InsertRequest().WithCollectionName(collection_name).WithRowsData(std::move(rows)), resp_insert);
    util::CheckStatus("insert", status);
    util::CheckStatus("insert", status);
    std::cout << "Successfully insert " << resp_insert.Results().InsertCount() << " rows." << std::endl;

    {
        // verify the row count by query(count(*))
        // set to STRONG level to ensure the delete request is done by server
        auto request = milvus::QueryRequest()
                           .WithCollectionName(collection_name)
                           .AddOutputField("count(*)")
                           .WithConsistencyLevel(milvus::ConsistencyLevel::STRONG);

        milvus::QueryResponse response;
        status = client->Query(request, response);
        util::CheckStatus("query count(*)", status);
        std::cout << "count(*) = " << response.Results().GetRowCount() << std::endl;
    }

    {
        // query with filter expression
        auto request = milvus::QueryRequest()
                .WithCollectionName(collection_name)
                .AddOutputField("*")
                .WithFilter(field_id + " in [5, 10]")
                // set to eventually level since the previous query already use strong level to ensure data is consumed
                .WithConsistencyLevel(milvus::ConsistencyLevel::EVENTUALLY);

        std::cout << "\nQuery with filter: " << request.Filter() << std::endl;
        milvus::QueryResponse response;
        status = client->Query(request, response);
        util::CheckStatus("query", status);

        milvus::EntityRows output_rows;
        status = response.Results().OutputRows(output_rows);
        util::CheckStatus("get output rows", status);
        std::cout << "Query results:" << std::endl;
        for (const auto& row : output_rows) {
            std::cout << "\t" << row << std::endl;
        }
    }

    {
        // do search
        auto request = milvus::SearchRequest()
                           .WithCollectionName(collection_name)
                           .WithFilter(field_age + " > 50")
                           .WithLimit(10)
                           .WithAnnsField(field_embedding)
                           .AddOutputField(field_name)
                           .AddOutputField(field_age)
                           .AddFloatVector(util::GenerateFloatVector(dimension))
                           .AddFloatVector(util::GenerateFloatVector(dimension))
                           .WithConsistencyLevel(milvus::ConsistencyLevel::BOUNDED);

        std::cout << "\nSearch with filter: " << request.Filter() << std::endl;
        milvus::SearchResponse response;
        status = client->Search(request, response);
        util::CheckStatus("search", status);

        auto search_results = response.Results();
        // get the result as row-based(recommended way)
        {
            std::cout << "Result of the first target vector:" << std::endl;
            const auto& result = search_results.Results().at(0);
            milvus::EntityRows output_rows;
            status = result.OutputRows(output_rows);
            util::CheckStatus("get output rows", status);
            for (const auto& row : output_rows) {
                std::cout << "\t" << row << std::endl;
            }
        }
    }

    // release collection
    status = client->ReleaseCollection(milvus::ReleaseCollectionRequest().WithCollectionName(collection_name));
    util::CheckStatus("release collection " + collection_name, status);

    // drop index
    status = client->DropIndex(milvus::DropIndexRequest().WithCollectionName(collection_name).WithFieldName(field_embedding));
    util::CheckStatus("drop index for field " + field_embedding, status);

    // drop collection
    status = client->DropCollection(milvus::DropCollectionRequest().WithCollectionName(collection_name));
    util::CheckStatus("drop collection " + collection_name, status);

    status = client->Disconnect();
    util::CheckStatus("disconnect to milvus server", status);
    return 0;
}
