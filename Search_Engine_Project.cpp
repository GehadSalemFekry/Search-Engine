#include <iostream> 
#include <fstream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <vector>
using namespace std;

struct link_data {
    int impressions;
    int clicks;
    double score;
    double CTR;
    double rank;
    vector<string> keywords;
};

unordered_map<string, vector<string>> adjacency;
unordered_map<string, link_data> web_info;

vector<string> split_string(string x, char sep) {
    vector<string> strings;
    string cur = "";
    for (auto ch : x) {
        if (ch == sep)
            strings.push_back(cur), cur = "";
        else
            cur += ch;
    }
    strings.push_back(cur);

    return strings;
}

// A general reading from file function
// Purpose (1: reading the graph, 2: reading the keywords, 3: reading the impressions, 4: reading the clicks)
void read_from_file(string filename, int purpose) {
    fstream fin;
    fin.open(filename);

    if (fin.fail()) {
        cout << "Sorry, cannot open the file, try again later!\n";
        exit(0);
    }

    while (!fin.eof()) {
        string line;
        getline(fin, line);
        vector<string> cur_line = split_string(line, ',');
        for (int i = 1; i < cur_line.size(); i++)
            if (purpose == 1)
                adjacency[cur_line[0]].push_back(cur_line[i]);
            else if (purpose == 2)
                web_info[cur_line[0]].keywords.push_back(cur_line[i]);
            else if (purpose == 3) // This will happen only one time (cur_line will be of length 2)
                web_info[cur_line[0]].impressions = stoi(cur_line[i]);
            else if (purpose == 4)
                web_info[cur_line[0]].clicks = stoi(cur_line[i]);
    }

    fin.close();
}

// Reading all the different files
void read_files() {
    read_from_file("web_graph.csv", 1); // Reading the graph
    read_from_file("keyword.csv", 2); // Reading the keywords associated with each link
    read_from_file("num_impressions.csv", 3); // Reading the number of impressions for each link
    read_from_file("clicks.csv", 4); // Reading the number of clicks for each link
}

unordered_map<string, bool> visited;
unordered_map<string, double> prev_rank;
void dfs(string cur) {
    visited[cur] = true;
    for (auto next : adjacency[cur]) {
        web_info[next].rank += prev_rank[cur] / adjacency[cur].size();

        if (!visited[next]) dfs(next);
    }
}

void create_page_rank() {
    // Initialize the rank of all pages
    for (auto& link : web_info)
        link.second.rank = 1.0 / web_info.size();

    const int MAX_ITERATIONS = 1e5;
    const int EPSILON = 1e-5;

    for (int i = 0; i < MAX_ITERATIONS; i++) {
        visited.clear();

        // Copy the rank and clear the current rank to be able to update it
        for (auto& link : web_info) {
            prev_rank[link.first] = link.second.rank;
            link.second.rank = 0;
        }

        // Traverse all node to update their page rank
        for (auto link : web_info)
            if (!visited[link.first])
                dfs(link.first);

        bool changed = false;
        for (auto link : web_info)
            if (abs(link.second.rank - prev_rank[link.first]) > EPSILON) changed = true;

        if (!changed) break;
    }
    // Vector holding rank and link to sort
    vector<pair<double, string>> links;
    for (auto link : web_info)
        links.push_back({ link.second.rank, link.first });

    // Updating the final rank of the link to be an index from a sorted ranks list  (i + 1 is the sorted rank index) 
    // Normalizing based on min_max normalization
    sort(links.begin(), links.end());
    for (int i = 0; i < links.size(); i++)
        web_info[links[i].second].rank = ((i + 1) - 1.0) / (web_info.size() - 1); // new_value = (value - min) / (max - min)
}

void create_score() {
    for (auto& link : web_info)
        link.second.score = (0.4 * link.second.rank) +
        ((1 - (0.1 * link.second.impressions / (1 + 0.1 * link.second.impressions))) * link.second.rank +
            (0.1 * link.second.impressions / (1 + 0.1 * link.second.impressions)) * link.second.CTR) * 0.6;
}

void create_CTR() {
    for (auto& link : web_info)
        link.second.CTR = 1.0 / link.second.impressions;
}

void initialize_graph_data() {
    create_CTR();
    create_score();
    create_page_rank();
}

void update_files() {
    ofstream fout;

    // Update Impressions
    fout.open("num_impressions.csv");
    for (auto link : web_info)
        fout << link.first << ',' << link.second.impressions << "\n";
    fout.close();

    // Update Clicks
    fout.open("clicks.csv");
    for (auto link : web_info)
        fout << link.first << ',' << link.second.clicks << "\n";
    fout.close();
}

void update_variables() {
    create_CTR();
    create_score();
    update_files();
}

void search_page();
void main_page();

vector<string> ans_websites;
void results_page() {
    if (ans_websites.size() == 0) {
        cout << "No Webpages found\n";
        main_page();
    }

    // Results
    cout << "Search Results:\n";

    for (int i = 0; i < ans_websites.size(); i++)
        cout << i + 1 << '.' << ans_websites[i] << "\n";

    cout << "Would you like to:\n"
        << "1.Choose a Webpage to open\n"
        << "2.New Search\n"
        << "3.Exit\n";

    int choice; cin >> choice;
    while (choice != 1 && choice != 2 && choice != 3) {
        cout << "Please enter a valid choice\n";
        cin >> choice;
    }

    if (choice == 1) {
        cout << "Please enter the index of your chosen webpage\n";
        int web_choice; cin >> web_choice;
        while (web_choice <= 0 || web_choice > ans_websites.size()) {
            cout << "Please enter a valid choice\n";
            cin >> web_choice;
        }

        string web_selected = ans_websites[web_choice - 1];
        web_info[web_selected].clicks++;
        update_variables();

        cout << "You are now viewing " << web_selected << "\n";
        cout << "Would you like to\n"
            << "1.Return to Search Results\n"
            << "2.New Search\n"
            << "3.Exit\n";

        int new_choice; cin >> new_choice;
        while (new_choice != 1 && new_choice != 2 && new_choice != 3) {
            cout << "Please enter a valid choice\n";
            cin >> new_choice;
        }

        if (new_choice == 1) results_page();
        else if (new_choice == 2) search_page();
        else exit(0);
    }
    else if (choice == 2) search_page();
    else exit(0);
}

void search_page() {
    cout << "Search:\n";

    string query;
    cin.ignore();
    getline(cin, query);

    int type = 3; // or
    vector<string> words;
    ans_websites.clear();

    if (query[0] == '"') {
        // Check if this string is found as it is
        query = query.substr(1, query.size() - 2);

        for (auto link : web_info) {
            if (count(link.second.keywords.begin(), link.second.keywords.end(), query) > 0) {
                ans_websites.push_back(link.first);
                web_info[link.first].impressions++;
            }
        }

    }
    else {
        transform(query.begin(), query.end(), query.begin(), ::tolower); // Transform the whole string to lower case

        words = split_string(query, ' ');

        for (auto& word : words) // words are all lower case
            if (word == "and") type = 2;
            else if (word == "or") type = 3;
    }

    if (type == 2) {
        // Check if all words exist in one or more links

        for (auto link : web_info) {
            vector<string> cur_keywords(link.second.keywords.begin(), link.second.keywords.end());
            for (auto key : cur_keywords)
                transform(key.begin(), key.end(), key.begin(), ::tolower);

            bool found = true;

            for (auto word : words)
                if (count(cur_keywords.begin(), cur_keywords.end(), word) == 0)
                    found = false;

            if (found) {
                ans_websites.push_back(link.first);
                web_info[link.first].impressions++;
            }
        }
    }
    else if (type == 3) {
        // Check if any word exists in one or more links

        for (auto link : web_info) {
            vector<string> cur_keywords(link.second.keywords.begin(), link.second.keywords.end());
            for (auto key : cur_keywords)
                transform(key.begin(), key.end(), key.begin(), ::tolower);

            for (auto word : words) {
                if (count(cur_keywords.begin(), cur_keywords.end(), word) > 0) {
                    ans_websites.push_back(link.first);
                    web_info[link.first].impressions++;
                    break;
                }
            }
        }
    }

    // Update with new Impressions
    update_variables();

    // Sort websites by their score
    sort(ans_websites.begin(), ans_websites.end(), [&](string link1, string link2) {
        return web_info[link1].score < web_info[link2].score;
        });

    results_page();
}

void main_page() {
    cout << "1.New Search\n"
        << "2.Exit\n"
        << "Type your choice: \n";

    int choice; cin >> choice;
    while (choice != 1 && choice != 2) {
        cout << "Please enter a valid choice\n";
        cin >> choice;
    }

    if (choice == 1) search_page();
    else exit(0); // it has to be two
}

void menu() {
    initialize_graph_data();

    cout << "Welcome!\n"
        << "What would you like to do?\n";
    main_page();
}


int main() {

    read_files();
    menu();

    return 0;
}