#include<iostream>
#include<chrono>
#include<vector>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<fstream>
#include<shared_mutex>
#include<queue>

using namespace std;
using namespace literals::chrono_literals;
using namespace chrono;
//some inits
const int MaxTimePart{ 180000 }, MaxTimeProduct{ 200000 };
int Products{ 0 };
ofstream out;
once_flag Open_flag;
mutex m1,m3;
shared_mutex m2;
condition_variable cv1, cv2;

ostream& operator<<(ostream& os, const vector<int>& A);

class Q {
	mutex m1;
	vector<int> Buffer{0,0,0,0,0};
	condition_variable cv1, cv2;

public:
	int max(int i, int j) {
		if (i > j) return i;
		else if (i <= j) return j;
	}
	//max of 2 numbers
	int min(int i, int j) {
		if (i < j) return i;
		else if (i >= j) return j;
	}
	//min of 2 numbers
	int vectorSum(vector<int>& A) {
		int i{ 0 };
		for (auto& j : A) {
			i = i + j;
		}
		return i;
	}
	//Sum of all integer elements of a vector
	void BufferTransfer(vector<int>& A) {
		if (A[0] + Buffer[0] <= 5) { Buffer[0] += A[0]; A[0] = 0; }
		else {
			A[0] = A[0] + Buffer[0] - 5;
			Buffer[0] = 5;
		}
		if (A[1] + Buffer[1] <= 5) { Buffer[1] += A[1]; A[1] = 0; }
		else {
			A[1] = A[1] + Buffer[1] - 5;
			Buffer[1] = 5;
		}
		if (A[2] + Buffer[2] <= 4) { Buffer[2] += A[2]; A[2] = 0; }
		else {
			A[2] = A[2] + Buffer[2] - 4;
			Buffer[2] = 4;
		}
		if (A[3] + Buffer[3] <= 3) { Buffer[3] += A[3]; A[3] = 0; }
		else {
			A[3] = A[3] + Buffer[3] - 3;
			Buffer[3] = 3;
		}
		if (A[4] + Buffer[4] <= 3) { Buffer[4] += A[4]; A[4] = 0; }
		else {
			A[4] = A[4] + Buffer[4] - 3;
			Buffer[4] = 3;
		}
	}
	bool transfer_part(vector<int>& part) {
		bool a{ false }, b{ false }, c{ false }, d{ false }, e{ false };
		if (Buffer[0] < 5 && part[0]>0) a = true;
		if (Buffer[1] < 5 && part[1]>0) b = true;
		if (Buffer[2] < 4 && part[2]>0) c = true;
		if (Buffer[3] < 3 && part[3]>0) d = true;
		if (Buffer[4] < 3 && part[4]>0) e = true;
		return a || b || c || d || e;
	}
	bool transfer_check(vector<int>& pickup) {
		bool a{ false }, b{ false }, c{ false }, d{ false }, e{ false };
		if (pickup[0] != 0) { if (Buffer[0] != 0) a = true; }
		if (pickup[1] != 0) { if (Buffer[1] != 0) b = true; }
		if (pickup[2] != 0) { if (Buffer[2] != 0) c = true; }
		if (pickup[3] != 0) { if (Buffer[3] != 0) d = true; }
		if (pickup[4] != 0) { if (Buffer[4] != 0) e = true; }
		return a || b || c || d || e;
	}
	//other helper functions

	void Push(vector<int>& Load_order, vector<int>& load_cart, system_clock::time_point t1, int i1, int p) { //p is ID, i1 is iteration
		unique_lock<mutex> UG1(m1);
		system_clock::time_point t2{ system_clock::now() };
		system_clock::duration Delay{ t2 - t1 };
		out << "Current Time: " << chrono::duration_cast<microseconds>(Delay).count() << " us" << endl;
		out << "Iteration " << (i1 + 1) << endl;
		out << "Part Worker ID: " << p << endl;
		out << "Status: New Load Order" << endl;
		out << "Accumulated Wait Time: 0 us" << endl;
		out << "Buffer State: " << Buffer;
		out << "Load Order: " << Load_order;
		load_cart[0] = max((Buffer[0] + Load_order[0]) - 5, 0);
		load_cart[1] = max((Buffer[1] + Load_order[1]) - 5, 0);
		load_cart[2] = max((Buffer[2] + Load_order[2]) - 4, 0);
		load_cart[3] = max((Buffer[3] + Load_order[3]) - 3, 0);
		load_cart[4] = max((Buffer[4] + Load_order[4]) - 3, 0);
		for (int i = 0; i < 5; i++) {
			Buffer[i] = Buffer[i] + Load_order[i] - load_cart[i];
		}
		out << "Updated Buffer State: " << Buffer;
		out << "Updated Load Order:" << load_cart << endl;
		cv2.notify_one();
		bool flag = false;
		if (vectorSum(load_cart) > 0) {
			system_clock::time_point t11{ system_clock::now() };
			while (cv1.wait_for(UG1, chrono::microseconds(MaxTimePart)) != cv_status::timeout) {
				flag = false;
				bool part_transf{false};
				if (transfer_part(load_cart)) {
					part_transf = true;
					system_clock::time_point t3{ system_clock::now() };					
					system_clock::duration Delay{ t3 - t1 };
					system_clock::duration Delay1{ t3 - t11 };
					out << "Current Time: " << chrono::duration_cast<microseconds>(Delay).count() << " us" << endl;
					out << "Iteration " << (i1 + 1) << endl;
					out << "Part Worker ID: " << p << endl;
					out << "Status:Wakeup-Notified" << endl;
					out << "Accumulated Wait Time: " << chrono::duration_cast<microseconds>(Delay1).count() << " us" << endl;
					out << "Buffer State: " << Buffer;
					out << "Load Order: " << load_cart;
					BufferTransfer(load_cart);
					out << "Updated Buffer State: " << Buffer;
					out << "Updated Load Order: " << load_cart << endl;
				}
				if (part_transf) {
					cv2.notify_one();
					part_transf = false;
				}
				if (vectorSum(load_cart) == 0) { 
					flag = true; 
					cv2.notify_one();
					break; 
				}
			}
			//cv2.notify_one();
			if (!flag) {
				system_clock::time_point t4{ system_clock::now() };
				system_clock::duration Delay{ t4 - t1 };
				system_clock::duration Delay1{ t4 - t11 };
				out << "Current Time: " << chrono::duration_cast<microseconds>(Delay).count() << " us" << endl;
				out << "Iteration " << (i1 + 1) << endl;
				out << "Part Worker ID: " << p << endl;
				out << "Status:Wakeup-Timeout" << endl;
				out << "Accumulated Wait Time: " << chrono::duration_cast<microseconds>(Delay1).count() << " us" << endl;
				out << "Buffer State: " << Buffer;
				out << "Load Order: " << load_cart;
				//BufferTransfer(load_cart);//needs to be commented out later
				out << "Updated Buffer State: " << Buffer;
				out << "Updated Load Order: " << load_cart << endl;
				cv2.notify_one();
			}
		}
	}
	void Pop(vector<int>& local_state, vector<int>& product_cart, vector<int>& update_pickup, 
		vector<int>& local_use, system_clock::time_point t1, vector<int>& B, int i1, int p) {
		unique_lock<mutex> UG1(m1);
		system_clock::time_point t2{ system_clock::now() };
		system_clock::duration Delay{ t2 - t1 };
		for (int i = 0; i < 5; i++) {
			update_pickup[i] = max(B[i] - local_state[i], 0);
		}
		out << "Current Time: " << chrono::duration_cast<microseconds>(Delay).count() << " us" << endl;
		out << "Iteration " << (i1 + 1) << endl;
		out << "Product Worker ID: " << p << endl;
		out << "Status: New Pickup Order" << endl;
		out << "Accumulated Wait Time: 0 us" << endl;
		out << "Buffer State: " << Buffer;
		out << "Pickup Order: " << update_pickup;
		out << "Local State: " << local_state;
		out << "Cart State: " << product_cart;
		for (int i = 0; i < 5; i++) {
			//update_pickup[i] = max(B[i] - local_state[i], 0);
			//local_state[i] -= B[i] - update_pickup[i];
			local_use[i] = B[i] - update_pickup[i];
		}
		for (int i = 0; i < 5; i++) {
			product_cart[i] = min(Buffer[i], update_pickup[i]);
			Buffer[i] = max(Buffer[i] - update_pickup[i], 0); //correct
			update_pickup[i] -= product_cart[i];
		}
		
		out << "Updated Buffer State: " << Buffer;
		out << "Updated Pickup Order: " << update_pickup;
		out << "Updated Local State: " << local_state;
		out << "Updated Cart State: " << product_cart;
		bool flag = false;
		bool dummy = true;
		system_clock::time_point t22{ system_clock::now() };
		cv1.notify_one();
		if (vectorSum(update_pickup) > 0) {//if condition needed to check if change is possible
			out << "Total Completed Products: " << Products << endl << endl;
			system_clock::time_point t11{ system_clock::now() };
			while (cv2.wait_for(UG1, chrono::microseconds(MaxTimeProduct)) != cv_status::timeout) {
				flag = false;
				bool prod_flag{ false };
				if (transfer_check(update_pickup)) {
					system_clock::time_point t4{ system_clock::now() };
					prod_flag = true;
					system_clock::duration Delay3{ t4 - t1 };
					system_clock::duration Delay1{ t4 - t11 };
					out << "Current Time: " << chrono::duration_cast<microseconds>(Delay3).count() << " us" << endl;
					out << "Iteration " << (i1 + 1) << endl;
					out << "Product Worker ID: " << p << endl;
					out << "Status: Wakeup-Notified" << endl;
					out << "Accumulated Wait Time: " << chrono::duration_cast<microseconds>(Delay1).count() << " us" << endl;
					out << "Buffer State: " << Buffer;
					out << "Pickup Order: " << update_pickup;
					out << "Local State: " << local_state;
					out << "Cart State: " << product_cart;
					for (int i = 0; i < 5; i++) {
						product_cart[i] += min(Buffer[i], update_pickup[i]);
						//local_state[i] = max(update_pickup[i] - Buffer[i], 0);
						Buffer[i] = max(Buffer[i] - update_pickup[i], 0);
						//product_cart[i] += update_pickup[i] - local_state[i];
						update_pickup[i] = B[i] - local_use[i] - product_cart[i];
					}
					out << "Updated Buffer State: " << Buffer;
					out << "Updated Pickup Order: " << update_pickup;
					out << "Local State: " << local_state;
					out << "Updated Cart State: " << product_cart;
					
				}
				if (vectorSum(update_pickup) == 0) { //break from the while loop
					flag = true;
					break;
				}
				if (prod_flag) {
					prod_flag = false;
					out << "Total Completed Products: " << Products << endl << endl;
					cv1.notify_one();
				}
			}
			t22 = t11;
			//cv1.notify_one();
		}
		//cv1.notify_one();
		else if (vectorSum(update_pickup) == 0) {
			dummy = false;
			int j = product_cart[0] * 200 + product_cart[1] * 200 + product_cart[2] * 300 + product_cart[3] * 300 + product_cart[4] * 400;
			this_thread::sleep_for(chrono::microseconds(j));
			int j1 = product_cart[0] * 600 + product_cart[1] * 600 + product_cart[2] * 700 + product_cart[3] * 700 + product_cart[4] * 800;
			int j2 = local_use[0] * 600 + local_use[1] * 600 + local_use[2] * 700 + local_use[3] * 700 + local_use[4] * 800;
			this_thread::sleep_for(chrono::microseconds(j1 + j2));
			Products++;
			system_clock::time_point t3{ system_clock::now() };
			system_clock::duration Delay2{ t3 - t1 };
			out << "Current Time: " << chrono::duration_cast<microseconds>(Delay2).count() << " us" << endl;
			for (int i = 0; i < 5; i++) {
				local_state[i] -= local_use[i];
			}
			out << "Updated Local State:" << local_state;
			out << "Updated Cart State: " << "( 0 0 0 0 0 )" << endl;
			out << "Total Completed Products: " << Products << endl << endl;
			//out << "Product Order Complete" << endl;
		}
		//cv1.notify_one();
		if (flag && dummy) {
			for (int i = 0; i < 5; i++) {
				local_state[i] -= local_use[i];
			}
			int j = product_cart[0] * 200 + product_cart[1] * 200 + product_cart[2] * 300 + product_cart[3] * 300 + product_cart[4] * 400;
			this_thread::sleep_for(chrono::microseconds(j));
			int j1 = product_cart[0] * 600 + product_cart[1] * 600 + product_cart[2] * 700 + product_cart[3] * 700 + product_cart[4] * 800;
			int j2 = local_use[0] * 600 + local_use[1] * 600 + local_use[2] * 700 + local_use[3] * 700 + local_use[4] * 800;
			this_thread::sleep_for(chrono::microseconds(j1 + j2));
			Products++;
			system_clock::time_point t5{ system_clock::now() };
			system_clock::duration Delay5{ t5 - t1 };
			//system_clock::duration Delay6{ t5 - t11 };
			out << "Current Time: " << chrono::duration_cast<microseconds>(Delay5).count() << " us" << endl;
			out << "Updated Local State: " << local_state;
			out << "Updated Cart State: " << "( 0 0 0 0 0 )" << endl;
			//system_clock::time_point t3{ system_clock::now() };
			//system_clock::duration Delay2{ t3 - t1 };
			
			/*out << "Current Time: " << chrono::duration_cast<microseconds>(Delay2).count() << " us" << endl;
			out << "Updated Local State: " << local_state << endl;
			out << "Updated Cart State: " << "( 0,0,0,0,0 )" << endl;*/
			out << "Total Completed Products: " << Products << endl << endl;
		}
		else if (!flag && dummy) {
			system_clock::time_point t2{ system_clock::now() };
			system_clock::duration Delay7{ t2 - t1 };
			system_clock::duration Delay6{ t2 - t22 };
			out << "Current Time: " << chrono::duration_cast<microseconds>(Delay7).count() << " us" << endl;
			//out << "Product thread wakes up" << endl;
			out << "Iteration " << (i1 + 1) << endl;
			out << "Product Worker ID: " << p << endl;
			out << "Status: Wakeup-Timeout" << endl;
			out << "Accumulated Wait Time: " << chrono::duration_cast<microseconds>(Delay6).count() << "us" << endl;
			out << "Buffer State: " << Buffer;
			out << "Pickup Order: " << update_pickup;
			out << "Local State: " << local_state;
			out << "Cart State: " << product_cart;
			//for (int i = 0; i < 5; i++) {
			//	product_cart[i] += min(Buffer[i], update_pickup[i]);
			//	//local_state[i] = max(update_pickup[i] - Buffer[i], 0);
			//	Buffer[i] = max(Buffer[i] - update_pickup[i], 0);
			//	//product_cart[i] += update_pickup[i] - local_state[i];
			//	update_pickup[i] = B[i] - local_use[i] - product_cart[i];
			//}
			/*out << "Updated Buffer State: " << Buffer;
			out << "Updated Pickup Order: " << update_pickup;
			out << "Local State: " << local_state;
			out << "Cart State: " << product_cart;*/
			int j = product_cart[0] * 200 + product_cart[1] * 200 + product_cart[2] * 300 + product_cart[3] * 300 + product_cart[4] * 400;
			this_thread::sleep_for(chrono::microseconds(j));
			system_clock::time_point t23{ system_clock::now() };
			system_clock::duration Delay8{ t23 - t1 };
			out << "Current Time: " << chrono::duration_cast<microseconds>(Delay8).count() << " us" << endl;
			for (int i = 0; i < 5; i++) {
				local_state[i] += product_cart[i];
				product_cart[i] = 0;
			}
			out << "Updated Local State: " << local_state;
			out << "Updated Cart State: " << product_cart;
			out << "Total Completed Products: " << Products << endl << endl;
		}
		if (dummy) cv1.notify_one();
		//cv1.notify_one();
	}
};

void PUSH(Q& q, int id, system_clock::time_point t1);

void POP(Q& q, int id, system_clock::time_point t1); 

int main() {
	system_clock::time_point t1{ system_clock::now() };

	call_once(Open_flag, [&]() {out.open("log.txt"); });
	const int m = 20, n = 16; //m: number of Part Workers
	//n: number of Product Workers
	//cout << system_clock::now().time_since_epoch().count() << endl;
	Q q;
	//cout << system_clock::now().time_since_epoch().count() << endl;
	vector<thread> V1;
	vector<thread> V2;
	//cout << system_clock::now().time_since_epoch().count() << endl;
	for (int i = 0; i < m; ++i) V1.emplace_back(PUSH, ref(q), i + 1, t1);
	//cout << system_clock::now().time_since_epoch().count() << endl;
	for (int i = 0; i < n; ++i) V2.emplace_back(POP, ref(q), i + 1, t1);
	//cout << system_clock::now().time_since_epoch().count() << endl;
	for (auto& i : V1) i.join();
	//cout << system_clock::now().time_since_epoch().count() << endl;
	for (auto& i : V2) i.join();
	//cout << system_clock::now().time_since_epoch().count() << endl;

	out << "Finish!" << endl;
	cout << "Product numbers " << Products << endl;
	
	if (out.is_open()) { out.close(); }

	return 0;
}

ostream& operator<<(ostream& os, const vector<int>& A) {
	os << "( ";
	for (auto& i : A) {
		os << i << " ";
	}
	os << ")" << endl;
	return os;
}
void PUSH(Q& q, int id, system_clock::time_point t1) {
	//seed of current time
	vector<int> load_cart(5, 0);
	for (int i1 = 0; i1 < 5; i1++) {
		shared_lock<shared_mutex> SG(m2);
		vector<int> Load_order(5, 0);
		int ex_part = q.vectorSum(load_cart);
		for (int i = 0; i < 5 - ex_part; i++) {
			srand(system_clock::now().time_since_epoch().count());
			int j = rand() % 5;
			Load_order[j] = Load_order[j] + 1;
		}
		for (int i = 0; i < 5; i++) {
			Load_order[i] += load_cart[i];
		}
		int j = Load_order[0] * 500 + Load_order[1] * 500 + Load_order[2] * 600 + Load_order[3] * 600
			+ Load_order[4] * 700; //time to make the parts
		this_thread::sleep_for(chrono::microseconds(j));
		int k = Load_order[0] * 200 + Load_order[1] * 200 + Load_order[2] * 300 + Load_order[3] * 300
			+ Load_order[4] * 400; //time to move parts to buffer area
		this_thread::sleep_for(chrono::microseconds(k));
		//try pushing
		q.Push(Load_order, load_cart, t1, i1, id);
	}
}
void POP(Q& q, int id, system_clock::time_point t1) {
	vector<int> local_state(5, 0); //buffer state of product worker's cart if no items available
	for (int i1 = 0; i1 < 5; i1++) {
		vector<int> product_cart(5, 0), update_pickup(5, 0), local_use(5, 0);
		//generating random pickup order code
		shared_lock<shared_mutex> SG(m2);
		vector<int> B(5);
		srand(system_clock::now().time_since_epoch().count());
		int a1 = rand() % 2;
		if (a1 == 0) { //if only 2 elements have pickup parts
			srand(system_clock::now().time_since_epoch().count());
			int a2 = rand() % 4;
			a2 = a2 + 1;
			int a3 = rand() % 5;
			B[a3] = a2;
			a2 = 5 - a2;
			int a4 = rand() % 5;
			while (a3 == a4) { a4 = rand() % 5; }
			B[a4] = a2;
		}
		else if (a1 == 1) {//if 3 elements have pickup parts
			srand(system_clock::now().time_since_epoch().count());
			int a2 = rand() % 2;
			if (a2 == 0) { //3 1 1
				int a3 = rand() % 5;
				B[a3] = 3;
				int a4 = rand() % 5;
				while (a3 == a4) { a4 = rand() % 5; }
				B[a4] = 1;
				int a5 = rand() % 5;
				while (a5 == a3 || a5 == a4) { a5 = rand() % 5; }
				B[a5] = 1;
			}
			else if (a2 == 1) {//2 2 1
				srand(system_clock::now().time_since_epoch().count());
				int a3 = rand() % 5;
				B[a3] = 2;
				int a4 = rand() % 5;
				while (a3 == a4) { a4 = rand() % 5; }
				B[a4] = 2;
				int a5 = rand() % 5;
				while (a5 == a3 || a5 == a4) { a5 = rand() % 5; }
				B[a5] = 1;
			}
		}
		//random pickup order generation complete
		q.Pop(local_state, product_cart, update_pickup, local_use, t1, B, i1, id);
	}
}
