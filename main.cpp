#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
struct Document {
    int id;
    double relevance;
};

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        double tf = 1./words.size();
        for (const auto& word : words) {
            documents_with_TF_[word][document_id] += tf;
        } document_count_ += 1;
    } 

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
   
    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    map<string, map<int, double>> documents_with_TF_;
    set<string> stop_words_;
    int document_count_ = 0;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    Query ParseQuery(const string& text) const {
        Query query;
        for (string& word : SplitIntoWords(text)) {
            if (word[0] == '-') {
                word = word.substr(1);
                if (!stop_words_.count(word)) {
                    query.minus_words.insert(word);
                }
            } else if (!stop_words_.count(word)) {
                query.plus_words.insert(word);
            }
        }
        return query;
    }
    
    vector<Document> FindAllDocuments(const Query& query_words) const {
        vector<Document> matched_documents;
        Query query;
        map<int, double> relevance;
               
        for (const auto& [word, id_tf] : documents_with_TF_) {
            if (query_words.plus_words.count(word)) {
                double idf = log(static_cast<double> (document_count_) / documents_with_TF_.at(word).size());
                for(const auto& [id, tf] : id_tf) {
                    relevance[id] += tf*idf;
                }
            }
        }
        for (const auto& [word,id_tf] : documents_with_TF_) {
            if (query_words.minus_words.count(word)) {
                for (const auto& [id, tf] : id_tf) {
                    relevance.erase(id);
                }
            }
        }    
            for (const auto& document : relevance) {
                matched_documents.push_back({document.first,
                                             document.second});
            }
        return matched_documents;
    }

    
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}
