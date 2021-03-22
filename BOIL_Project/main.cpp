#include <iostream>
#include <vector>
#include <cstdio>
#include <fstream>
#include <sstream>

using namespace std;

const int deliverers = 2; // 3
const int receivers = 3; // 2
vector<float> supply; //podaz 
vector<float> demand; //popyt
vector<float> transport_cost;
vector<float> sale_price;
vector<float> purchase_cost;
float unitry_cost_trans[deliverers][receivers];
int block[2];
bool isBalanced();
void readFromTxt();
void saveToTxt();
float** unitry_profit_fun();
float** route_fun();
int checkDeliv(int temp);
float** unitry_profit;
float** route;

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

bool isBalanced() {
	float sum_supply = 0;
	float sum_demand = 0;

	for (auto i : supply) {
		sum_supply += i;
	}

	for (auto i : demand) {
		sum_demand += i;
	}

	return (sum_demand == sum_supply);
}

void saveToTxt() {
	cout << "SAVED";
}

void PrintForDebug() {

}

float** unitry_profit_fun() {

	//float** unitry_profit; 

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
		//add to matric fictional receiver and deliver
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
	for (int i = 0; i < deliverers + 1; i++) {
		for (int j = 0; j < receivers + 1; j++) {
			cout << route[i][j] << " ";
		}
		cout << endl;
	}
}

float** route_fun() {

	if (isBalanced()) {
		route = new float* [deliverers];
		for (int i = 0; i < deliverers; i++) {
			route[i] = new float[receivers];
			for (int j = 0; j < receivers; j++) {
				route[i][j] = 0;
			}
		}
	}
	else {
		//add to matric fictional receiver and deliver
		route = new float* [deliverers + 1];
		for (int i = 0; i < deliverers + 1; i++) {
			route[i] = new float[receivers + 1];
			for (int j = 0; j < receivers + 1; j++) {
				route[i][j] = 0;
			}
		}

		int temp = 0;
		for (int i = 0; i < deliverers; i++) {
				if (unitry_profit[i][block[1]] < unitry_profit[i + 1][block[1]])
					temp = i + 1;
		}
		
		
		cout << "S: " << supply[temp] << " D: " << demand[block[1]] << endl;
		if (supply[temp] >= demand[block[1]]) {
			route[temp][block[1]] = demand[block[1]];
			demand[block[1]] = 0;
			supply[temp] = supply[temp] - route[temp][block[1]];
			print(); 
		}
		else
		{
			route[temp][block[1]] = supply[temp];
			supply[temp] = 0;
			demand[block[1]] = demand[block[1]] - route[temp][block[1]];
			checkDeliv(temp);
			print(); 
		}
		
	}
	
	return route;
}

int checkDeliv(int temp) {	//przeszukaæ ponownie tablice za wyj¹tkiem tego temp co by³o wczeœnej
	int j = temp;
	for (int i = 0; i < deliverers; i++) {
		if (i != j)
		{
			if (unitry_profit[i][block[1]] < unitry_profit[i + 1][block[1]])
				temp = i + 1;
			else
				temp = i;
		}
	}
	return temp;
}