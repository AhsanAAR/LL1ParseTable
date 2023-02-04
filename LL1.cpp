#include <fstream>
#include <string>
#include <map>
#include <set>
#include <iostream>
#include <vector>

using namespace std;

vector<string> split(string s, string delimiter) {
	vector<string> ans;
	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		ans.push_back(token);
		s.erase(0, pos + delimiter.length());
	}
	ans.push_back(s);
	return ans;
}

pair<set<string>, bool> first(string s, map<string, vector<vector<string>>>& grammar, const set<string>& non_terminals, map<string, set<string>>& first_sets) {
	if (non_terminals.find(s) == non_terminals.end()) {
		return make_pair(set<string>({ s }), false);
	}
	else if (first_sets.find(s) != first_sets.end()) {
		if (first_sets[s].find("~") == first_sets[s].end()) {
			return make_pair(first_sets[s], false);
		}
		else {
			return make_pair(first_sets[s], true);
		}
	}

	set<string> ans;
	for (auto production : grammar[s]) {
		for (int i = 0; i < production.size(); i++) {
			auto f = first(production[i], grammar, non_terminals, first_sets);
			if (i < production.size() - 1) {
				f.first.erase("~");
			}
			ans.insert(f.first.begin(), f.first.end());

			if (!f.second)
				break;
		}
	}
	first_sets[s] = ans;

	if (ans.find("~") == ans.end()) {
		return make_pair(ans, false);
	}
	else {
		return make_pair(ans, true);
	}
}

bool start_follow_set = false;

set<string> follow(string s, map<string, vector<vector<string>>>& grammar, map<string, set<string>>& follow_sets, map<string, set<string>>& first_sets, const set<string>& non_terminals, string start_symbol) {
	if (follow_sets.find(s) != follow_sets.end()) {
		if (s != start_symbol || start_follow_set) {
			return follow_sets[s];
		}
	}

	set<string> ans;

	for (auto i = grammar.begin(); i != grammar.end(); i++) {
		for (auto production : i->second) {
			for (int l = 0; l < production.size(); l++) {
				if (production[l] == s) {
					for (int k = l+1; k <= production.size(); k++) {
						if (k == production.size()) {
							if (i->first != s) {
								auto f = follow(i->first, grammar, follow_sets, first_sets, non_terminals, start_symbol);
								ans.insert(f.begin(), f.end());
							}
							else {
								break;
							}
						}
						else {
							auto f = first(production[k], grammar, non_terminals, first_sets);
							f.first.erase("~");
							ans.insert(f.first.begin(), f.first.end());
							if (!f.second)
								break;
						}
					}
				}
			}
		}
	}

	follow_sets[s].insert(ans.begin(), ans.end());

	if (s == start_symbol) {
		start_follow_set = true;
	}

	return ans;
}

string production_to_string(vector<string> a) {
	string s;
	for (string c : a) {
		s += c;
	}
	return s;
}

int main22() {
	string file_name;
	cout << "Enter the grammar file name.";
	cin >> file_name;
	ifstream reader(file_name);
	string line;
	string start_symbol;
	set<string> terminals;
	set<string> non_terminals;
	map<string, vector<vector<string>>> grammar;
	map<string, vector<pair<int, int>>> terminal_indices;
	bool start_found = false;

	while (!reader.eof()) {
		getline(reader, line);
		auto production = split(line, "\t");
		string non_terminal = production[0];
		non_terminals.insert(non_terminal);

		if (!start_found) {
			start_symbol = non_terminal;
			start_found = true;
		}
		
		production.erase(production.begin());
		grammar[non_terminal].push_back(production);
	}

	for (auto i = grammar.begin(); i != grammar.end(); i++) {
		for (auto production : i->second) {
			for (auto lit : production) {
				if (non_terminals.find(lit) == non_terminals.end()) {
					terminals.insert(lit);
				}
			}
		}
	}
	terminals.erase("~");
	terminals.insert("$");

	map<string, set<string>> first_sets;
	map<string, set<string>> follow_sets;
	follow_sets[start_symbol].insert({ "$" });

	for (string non_terminal : non_terminals) {
		first(non_terminal, grammar, non_terminals, first_sets);
	}

	for (string non_terminal : non_terminals) {
		follow(non_terminal, grammar, follow_sets, first_sets, non_terminals, start_symbol);
	}

	first_sets;
	follow_sets;

	map < string, map<string, vector<string>>> parse_table;

	for (auto i = grammar.begin(); i != grammar.end(); i++) {
		string non_terminal = i->first;
		for (auto production : i->second) {
			auto first_s = first(production[0], grammar, non_terminals, first_sets);
			if (first_s.second) {
				auto follow_s = follow(non_terminal, grammar, follow_sets, first_sets, non_terminals, start_symbol);
				for (string lit : follow_s) {
					if (parse_table[non_terminal].find(lit) == parse_table[non_terminal].end()) {
						parse_table[non_terminal][lit] = vector<string>({ "~" });
					}
					else {
						cout << "Error LL1 repetition " << non_terminal << " " << lit;
					}
				}
			}
			else {
				for (string lit : first_s.first) {
					if (parse_table[non_terminal].find(lit) == parse_table[non_terminal].end()) {
						parse_table[non_terminal][lit] = production;
					}
					else {
						cout << "Error LL1 repetition " << non_terminal << " " << lit;
					}
				}
			}
		}
	}

	ofstream writer;
	
	writer.open("First.txt");
	for (string non_terminal : non_terminals) {
		writer << non_terminal;
		for (string lit : first_sets[non_terminal]) {
			writer << '\t' << lit;
		}
		writer << '\n';
	}
	writer.close();

	writer.open("Follow.txt");
	for (string non_terminal : non_terminals) {
		writer << non_terminal;
		for (string lit : follow_sets[non_terminal]) {
			writer << '\t' << lit;
		}
		writer << '\n';
	}
	writer.close();

	writer.open("LL1ParsingTable.csv");
	writer << "cols";
	for (string terminal : terminals) {
		writer << "," << terminal;
	}
	writer << endl;

	for (string non_terminal : non_terminals) {
		writer << non_terminal;
		for (string terminal : terminals) {
			writer << "," << production_to_string(parse_table[non_terminal][terminal]);
		}
		writer << endl;
	}

	return 0;
}