#include <iostream>
#include <vector>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <utility>

using namespace std;

const int deliverers = 2; // 3
const int receivers = 3; // 2

vector<float> supply; //podaz 
vector<float> demand; //popyt
vector<float> transport_cost;
vector<float> sale_price;
vector<float> purchase_cost;
vector<float> profits;

float unitry_cost_trans[deliverers][receivers];
int block[2];
float** unitry_profit;
float** route;
int** route_done;
float* alfa;
float* beta;
float** delta;

void readFromTxt();
bool isBalanced();
void saveToTxt();

void route_fun();
pair<int,int> checkDeliv(); // zwraca dwa inty - "wspó³rzedne" komórki
void init_alfa_beta();

void calculate_profit();
void calculate_alfa_beta();
bool calculate_delta();

float** unitry_profit_fun();
pair<int, int> bad_delta();
pair<int, int>* find_new_cycle(pair<int, int> bad_delta);
void change_unitary_route(pair<int, int>* cycle);

//prints
bool print_to_console = false; 
void print_delta();
void print_alfa_beta();
void print_supply_demand();
void print_route();

int main() {
	
	readFromTxt();
	unitry_profit_fun();
	route_fun();

	while(1) {
		calculate_profit();
		init_alfa_beta();
		calculate_alfa_beta();
		if (calculate_delta()) {
			break;
		}

		if (print_to_console) {
			print_delta();
			print_route();
		}

		change_unitary_route(find_new_cycle(bad_delta()));
	}
	saveToTxt();

	return 1;
}

void readFromTxt() {
	fstream file;
	file.open("data.txt", ios::in);

	float file_var;

	for (int i = 0; i < deliverers; i++) {
		file >> file_var;
		supply.push_back(file_var);
	}

	for (int i = 0; i < receivers; i++) {
		file >> file_var;
		demand.push_back(file_var);
	}


	for (int i = 0; i < receivers; i++) {
		file >> file_var;
		sale_price.push_back(file_var);
	}

	for (int i = 0; i < deliverers; i++) {
		file >> file_var;
		purchase_cost.push_back(file_var);
	}

	for (int i = 0; i < deliverers; i++) {
		for (int j = 0; j < receivers; j++) {
			file >> unitry_cost_trans[i][j];
		}
	}

	for (int i = 0; i < receivers; i++) {
		file >> file_var;
		transport_cost.push_back(file_var);
	}
	for (int i = 0; i < 2; i++) {
		file >> file_var;
		block[i]= file_var;

		if (print_to_console) {
			cout << block[i] << " " << endl;
		}
	}

	file.close();
}

bool isBalanced() { // zmiana na zwracania float'a z róznica - jak zwraca 0 to jest zbliansowany czyli w warunkach dodaje negacje
					// i odrazu do wektora dodaje "fikcyjny popyt i fikcyjn¹ poda¿".
	float sum_supply = 0;
	float sum_demand = 0;

	for (auto i : supply) {
		sum_supply += i;
	}

	for (auto i : demand) {
		sum_demand += i;
	}

	float difference = sum_demand - sum_supply;

	float fictional_demand = 50;
	float fictional_supply = 50;

	float try_fictional = sum_demand + fictional_demand - sum_supply;
	if (try_fictional > 0) {
		fictional_demand = try_fictional;
	}
	else {
		fictional_supply = fictional_supply* (-1) + 50;
	}


	supply.push_back(fictional_demand);
	demand.push_back(fictional_supply);

	return (sum_demand == sum_supply);
}

void saveToTxt() {
	ofstream save("wyniki.txt");
	save << "Profits: "<<endl;

	if (print_to_console) {
		cout << "Profits: " << endl;
	}

	for (auto i : profits) {

		if (print_to_console) {
			cout << i << " ";
		}
		save << i << " ";
	}
	save << endl;

	if (print_to_console) {
		cout << endl;
	}
	save.close();

	float max_profit = 0;
	for (auto i : profits) {
		if (max_profit < i) {
			max_profit = i;
		}
	}
	if (print_to_console) {
		cout << endl << "MAX Profit: " << max_profit << endl;
	}
}

float** unitry_profit_fun() {


	if (isBalanced()) {
		
		unitry_profit = new float* [deliverers];
		for (int i = 0; i < deliverers; i++) {
			unitry_profit[i] = new float[receivers];
			for (int j = 0; j < receivers; j++) {
				unitry_profit[i][j] = sale_price[j] - purchase_cost[i] - unitry_cost_trans[i][j];
			}
		}
	}
	else {
		//add to matrix fictional receiver and deliver
		unitry_profit = new float* [deliverers+1];
		for (int i = 0; i < deliverers+1; i++) {
			unitry_profit[i] = new float[receivers+1];
			for (int j = 0; j < receivers+1; j++) {
				unitry_profit[i][j] = 0;
			}
		}

		for (int i = 0; i < deliverers; i++) {
			for (int j = 0; j < receivers; j++) {
				unitry_profit[i][j] = sale_price[j] - purchase_cost[i] - unitry_cost_trans[i][j];
			}
		}
	}

	if (block[0] != -1)
		unitry_profit[block[0]][block[1]] = -100;

	// tylko dla nie zbilansowanego!!!!
	for (int i = 0; i < deliverers + 1; i++) {
		for (int j = 0; j < receivers + 1; j++) {

			if (print_to_console) {
				cout << unitry_profit[i][j] << " ";
			}
		}
		if (print_to_console) {
			cout << endl;
		}
	}

	return unitry_profit;
}

void print_route() {


	cout << endl << "Route" << endl;
	for (int i = 0; i < deliverers + 1; i++) {
		for (int j = 0; j < receivers + 1; j++) {
			cout << route[i][j] << " ";
		}
		cout << endl;
	}

	cout << endl << "Route done" << endl;
	for (int i = 0; i < deliverers + 1; i++) {
		for (int j = 0; j < receivers + 1; j++) {
			cout << route_done[i][j] << " ";
		}
		cout << endl;
	}
}

void route_fun() {

	int supply_size = sizeof(supply)/sizeof(float);
	int demand_size = sizeof(demand) / sizeof(float);
	// zrobi³em w ten sposób bo przy alokacji za pomoca zwyk³ego supply.size() by³y jakieœ krzaczki z rozmiarem pamiêci

	route = new float* [supply_size];
	route_done = new int* [supply_size];
	for (int i = 0; i < supply.size(); i++) {
		route[i] = new float[demand_size];
		route_done[i] = new int[demand_size];
		for (int j = 0; j < demand.size(); j++) {
			route_done[i][j] = 0;
			route[i][j] = 0;
		}
	}

	route_done[block[0]][block[1]] = 1; // odrazu zablokowanej trasie uniemozliwenie sprawdzenia, sama trasa i tak ma 0;

	for (int i = 0; i < supply.size() * demand.size() - 1 ; i++) { //pêtla po wzsystkich elementach w tablicy
		pair<int, int> cell_to_process = checkDeliv(); //zwracanie komórki, która ma byæ przetwarzana

		if (print_to_console) {
			cout << "Cell: " << cell_to_process.first << ", " << cell_to_process.second << endl;
			cout << "S: " << supply[cell_to_process.first] << " D: " << demand[cell_to_process.second] << endl;
		}

		if (supply[cell_to_process.first] >= demand[cell_to_process.second]) {
			route[cell_to_process.first][cell_to_process.second] = demand[cell_to_process.second];
			supply[cell_to_process.first] = supply[cell_to_process.first] - demand[cell_to_process.second];
			demand[cell_to_process.second] = 0;

			if (print_to_console) {
				print_route();
			}
		}
		else
		{
			route[cell_to_process.first][cell_to_process.second] = supply[cell_to_process.first];
			supply[cell_to_process.first] = 0;
			demand[cell_to_process.second] = demand[cell_to_process.second] - route[cell_to_process.first][cell_to_process.second];

			if (print_to_console) {
				print_route();
			}

		}
		if (print_to_console) {
			print_supply_demand();
		}
	}
}

pair<int, int> checkDeliv() {	//przeszukaæ ponownie tablice za wyj¹tkiem tego temp co by³o wczeœnej

	// najpierw zablokowane trasy
	int temp_supply = -1;
	int temp_demand = block[1];
	float profit_value_max = -1000000000000; //g³upie wiem

	for (int i = 0; i < supply.size(); i++) {
		if (route_done[i][block[1]] == 0) {
			if (profit_value_max < unitry_profit[i][block[1]]) {
				temp_supply = i;
				profit_value_max = unitry_profit[i][block[1]];

			}	
		}
	}

	if (temp_supply != -1) { // jak jest rowne -1 to znaczy ¿e nie zosta³o przydzielone czyli odbiorca z priorytetem przydzielony
		route_done[temp_supply][temp_demand] = 1;
		return pair<int, int>(temp_supply, temp_demand);
	}
	temp_demand = -1;

	for (int i = 0; i < supply.size(); i++) {
		for (int j = 0; j < demand.size(); j++) {
			if (route_done[i][j] == 0) {
				if (profit_value_max < unitry_profit[i][j]) {
					temp_supply = i;
					temp_demand = j;
					profit_value_max = unitry_profit[i][j];
				}
			}
		}
	}

	route_done[temp_supply][temp_demand] = 1;
	return pair<int, int>(temp_supply, temp_demand);
}

void print_supply_demand() {
	
	cout << "Suppliers: ";
	for (auto i : supply) {
		cout << i << " ";
	}
	cout << endl;

	cout << "Receivers: ";
	for (auto i : demand) {
		cout << i << " ";
	}
	cout << endl;
}

void calculate_profit() {
	float profit = 0;

	for (int i = 0; i < supply.size(); i++) {
		for (int j = 0; j < demand.size(); j++) {
			profit += unitry_profit[i][j] * route[i][j];
		}
	}
	
	profits.push_back(profit);

	if (print_to_console) {
		cout << "Profits: ";
		for (auto i : profits) {
			cout << i << " ";
		}
		cout << endl;
	}
}

void init_alfa_beta() {

	int supply_size = sizeof(supply) / sizeof(float);
	int demand_size = sizeof(demand) / sizeof(float);

	alfa = new float[supply_size];
	beta = new float[supply_size];

	for (int i = 0; i < supply_size; i++) {
		alfa[i] = -1000000000;
	}

	for (int i = 0; i < demand_size; i++) {
		beta[i] = -1000000000;
	}

	alfa[0] = 0;
}

void calculate_alfa_beta() {

	int next_index_to_calculate = -1;
	float try_ = 0;
	for (int i = 0; i < supply.size(); i++) {
		for (int j = 0; j < demand.size(); j++) {

			if (route[i][j] != 0) {
				try_ = unitry_profit[i][j];

				if (beta[j] == -1000000000) {
					beta[j] = try_ - alfa[i];
					next_index_to_calculate = j;
				}
			}
		}

		for (int j = 0; j < supply.size(); j++) {
			if (print_to_console) {
				cout << next_index_to_calculate << " , " << j << endl;
			}
			if (route[j][next_index_to_calculate] != 0) {
				try_ = unitry_profit[j][next_index_to_calculate];

				if (alfa[j] == -1000000000) {
					alfa[j] = try_ - beta[next_index_to_calculate];
				}
			}
		}
		if (print_to_console) {
			print_alfa_beta();
		}
	}

}

void print_alfa_beta() {

	cout << "alfa: ";
	for (int i = 0; i < supply.size(); i++) {
		cout << alfa[i] << " ";
	}
	cout << endl;

	cout << "beta: ";
	for (int i = 0; i < demand.size(); i++) {
		cout << beta[i] << " ";
	}
	cout << endl;
}

bool calculate_delta() {

	bool final_result = true;
	int supply_size = sizeof(supply) / sizeof(float);
	int demand_size = sizeof(demand) / sizeof(float);
	// za du¿o tych alokacji - trzeba to wszystko zrobiæ w jednej funkcji

	delta = new float* [supply_size];

	for (int i = 0; i < supply.size(); i++) {
		delta[i] = new float[demand_size];

		for (int j = 0; j < demand.size(); j++) {

			if (route[i][j] == 0) {
				delta[i][j] = unitry_profit[i][j] - alfa[i] - beta[j];
				if (delta[i][j] > 0) {
					final_result = false;
				}
			} 
			else {
				delta[i][j] = 0;
			}
		}
	}
	return final_result;
}

void print_delta() {

	cout << endl << "Delta" << endl;
	for (int i = 0; i < supply.size() ; i++) {
		for (int j = 0; j < demand.size(); j++) {
			cout << delta[i][j] << " ";
		}
		cout << endl;
	}

}

pair<int, int> bad_delta() {

	if (print_to_console) {
		cout << endl << " Bad delta" << endl;
	}
	for (int i = 0; i < supply.size(); i++) {
		for (int j = 0; j < demand.size(); j++) {

			if (delta[i][j] > -1) {

				if (print_to_console) {
					cout << delta[i][j] << " ";
				}
				return pair<int, int>(i, j);
			}
		}

		if (print_to_console) {
			cout << endl;
		}
	}
	
	return pair<int, int>(-1, -1);
}

pair<int, int>* find_new_cycle(pair<int, int> bad_delta) {

	pair<int, int>* cycle_to_change = new pair<int, int>[4];

	if (bad_delta.first == -1 && bad_delta.second == -1) {
		return cycle_to_change;
	}

	cycle_to_change[0] = bad_delta;

	for (int k = 0; k < supply.size() * demand.size() - 1; k++) {

		for (int j = 0; j < demand.size(); j++) {
			if (delta[bad_delta.first][j] == 0) {
				cycle_to_change[1] = pair<int, int>(bad_delta.first, j);
			}
		}

		
		for (int j = 0; j < supply.size(); j++) {
			if (delta[j][bad_delta.second] == 0) {
				cycle_to_change[2] = pair<int, int>(j, bad_delta.second);
			}
		}

		if (delta[cycle_to_change[1].second][cycle_to_change[2].first] == 0) {
			cycle_to_change[3] = pair<int,int> (cycle_to_change[2].first, cycle_to_change[1].second);
		}
	}

	if (print_to_console) {
		cout << endl << "Cycle to change" << endl;
		for (int j = 0; j < 4; j++) {
			cout << cycle_to_change[j].first << cycle_to_change[j].second << endl;
		}
	}

	return cycle_to_change;
}

void change_unitary_route(pair<int, int>* cycle) {

	float value_to_cycle; 
	if (route[cycle[1].first][cycle[1].second] <= route[cycle[2].first][cycle[2].second]) {
		value_to_cycle = route[cycle[1].first][cycle[1].second];
	}
	else {
		value_to_cycle = route[cycle[2].first][cycle[2].second];
	}


	route[cycle[0].first][cycle[0].second] = route[cycle[0].first][cycle[0].second] + value_to_cycle;
	route[cycle[2].first][cycle[2].second] = route[cycle[2].first][cycle[2].second] - value_to_cycle;

	route[cycle[1].first][cycle[1].second] = route[cycle[1].first][cycle[1].second] - value_to_cycle;
	route[cycle[3].first][cycle[3].second] = route[cycle[3].first][cycle[3].second] + value_to_cycle;

	if (print_to_console) {
		print_route();
	}
}