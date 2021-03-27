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

bool isBalanced();
void readFromTxt();
void saveToTxt();
float** unitry_profit_fun();
float** route_fun();
pair<int,int> checkDeliv(); // zwraca dwa inty - "wspó³rzedne" komórki
void print_supply_demand();
void calculate_profit();
float** unitry_profit;
float** route;
int** route_done;


int main() {
	
	readFromTxt();
	unitry_profit_fun();
	route_fun();
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
		cout << block[i] << " " << endl;
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
	cout << "SAVED";
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
			cout << unitry_profit[i][j] << " ";
		}
		cout << endl;
	}


	return unitry_profit;
}

void print() {

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

float** route_fun() {

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

		cout << "Cell: " << cell_to_process.first << ", " << cell_to_process.second << endl;
		cout << "S: " << supply[cell_to_process.first] << " D: " << demand[cell_to_process.second] << endl;

		if (supply[cell_to_process.first] >= demand[cell_to_process.second]) {
			route[cell_to_process.first][cell_to_process.second] = demand[cell_to_process.second];
			supply[cell_to_process.first] = supply[cell_to_process.first] - demand[cell_to_process.second];
			demand[cell_to_process.second] = 0;
			print();
		}
		else
		{
			route[cell_to_process.first][cell_to_process.second] = supply[cell_to_process.first];
			supply[cell_to_process.first] = 0;
			demand[cell_to_process.second] = demand[cell_to_process.second] - route[cell_to_process.first][cell_to_process.second];
			print();

		}
		print_supply_demand();
	}
	calculate_profit();

	return route;
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
	cout << "Profits: ";
	for (auto i : profits) {
		cout << i << " ";
	}
	cout << endl;
}