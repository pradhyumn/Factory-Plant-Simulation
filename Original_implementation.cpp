#include<iostream>
#include<chrono>
#include<vector>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<fstream>
#include<shared_mutex>

using namespace std;
using namespace literals::chrono_literals;
using namespace chrono;

//Inits
const int MaxTimePart{ 1800 }, MaxTimeProduct{ 2000 };
int seed{ 100 };

vector<int> Buffer{ 0,0,0,0,0 };
int Products{ 0 };
ofstream out;

mutex m11;
shared_mutex m1, m2;
condition_variable cv1, cv2;
//Boolean
once_flag Open_flag;
system_clock::time_point t1{ system_clock::now() };

void PartWorker(int p);
void ProductWorker(int p);

int max(int i, int j);
int min(int i, int j);
int vectorSum(vector<int>& A);
void BufferTransfer(vector<int>& A);
bool transfer_check(vector<int>& pickup);
bool transfer_part(vector<int>& part);
ostream& operator<<(ostream& os, const vector<int>& A);

int main() {
	const int m = 20, n = 16; //m: number of Part Workers
	//n: number of Product Workers
	//Different numbers might be used during grading.
	
	vector<thread> PartW, ProductW;
	//ofstream out("log.txt");
	for (int i = 0; i < m; ++i) {
		PartW.emplace_back(PartWorker, i + 1);
	}
	for (int i = 0; i < n; ++i) {
		ProductW.emplace_back(ProductWorker, i + 1);
	}

	for (auto& i : PartW) i.join();
	for (auto& i : ProductW) i.join();

	out << "Finish!" << endl;
	cout << "Product numbers " << Products << endl;
	if (out.is_open()) { out.close(); }
	return 0;
}
int max(int i, int j) {
	if (i > j) return i;
	else if (i <= j) return j;
}
int min(int i, int j) {
	if (i < j) return i;
	else if (i >= j) return j;
}
int vectorSum(vector<int>& A) {
	int i{ 0 };
	for (auto& j : A) {
		i = i + j;
	}
	return i;
}
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
ostream& operator<<(ostream& os, const vector<int>& A) {
	os << "( ";
	for (auto& i : A) {
		os << i << " ";
	}
	os << ")" << endl;
	return os;
}
void PartWorker(int p) {
	
	vector<int>load_cart(5, 0); //already existing products which couldn't be moved to buffer in previous iteration
	for (int i1 = 0; i1 < 5; i1++) {
		vector<int> load_order_update(5, 0);
		{
			lock_guard<shared_mutex> LG1(m1);
			srand(seed++);
		}
		shared_lock<shared_mutex> SG(m2);
		vector<int> Load_order(5);
		int ex_part = vectorSum(load_cart);
		for (int i = 0; i < 5 - ex_part; i++) {
			int j = rand() % 5;
			Load_order[j] = Load_order[j] + 1;
		}
		for (int i = 0; i < 5; i++) {
			Load_order[i] += load_cart[i];
		}
		int j = Load_order[0] * 500 + Load_order[1] * 500 + Load_order[2] * 600 + Load_order[3] * 600 + Load_order[4] * 700; //time to make the parts
		this_thread::sleep_for(chrono::microseconds(j));
		int k = Load_order[0] * 200 + Load_order[1] * 200 + Load_order[2] * 300 + Load_order[3] * 300 + Load_order[4] * 400; //time to move parts to buffer area
		this_thread::sleep_for(chrono::microseconds(k));
		{
			unique_lock<mutex> LG1(m11);
			call_once(Open_flag, [&]() {out.open("log.txt"); });
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
				//load_order_update[i]=
			}
			out << "Updated Buffer State: " << Buffer;
			out << "Updated Load Order:" << load_cart << endl;
			cv2.notify_all();
			bool flag = false;
			//out << "-------------------------------------" << endl;
			//out << "Vector Sum = " << vectorSum(load_cart) << endl;
			system_clock::time_point t11{ system_clock::now() };
			if (vectorSum(load_cart) > 0) {
				while (cv1.wait_for(LG1, chrono::microseconds(MaxTimePart)) != cv_status::timeout) {
					flag = false;
					if (transfer_part(load_cart)) {
						out << "Sleeping thread wakes up" << endl;
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
					if (vectorSum(load_cart) == 0) { flag = true; break; }
				}
				cv2.notify_all();
				if (flag) { 
					continue;
					//out << "-----------Sleeping thread woken up------" << endl; 
				}
				else if (!flag) { 
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
					BufferTransfer(load_cart);
					out << "Updated Buffer State: " << Buffer;
					out << "Updated Load Order: " << load_cart << endl;
					//out << "---------------Sleeping thread timed out-----------" << endl; 
				}
				cv2.notify_all();
			}
		}
	}
}

void ProductWorker(int p) {
	vector<int> local_state(5, 0); //buffer state of product worker's cart if no items available
	for (int i1 = 0; i1 < 5; i1++) {
		vector<int> product_cart(5, 0), update_pickup(5, 0), local_use(5, 0);
		{
			lock_guard<shared_mutex> LG1(m1);
			srand(seed++);
		}
		//generating random pickup order code
		shared_lock<shared_mutex> SG(m2);
		vector<int> B(5);
		int a1 = rand() % 2;
		if (a1 == 0) {
			int a2 = rand() % 4;
			a2 = a2 + 1;
			int a3 = rand() % 5;
			B[a3] = a2;
			a2 = 5 - a2;
			int a4 = rand() % 5;
			while (a3 == a4) { a4 = rand() % 5; }
			B[a4] = a2;
		}
		else if (a1 == 1) {
			int a2 = rand() % 2;
			if (a2 == 0) {
				int a3 = rand() % 5;
				B[a3] = 3;
				int a4 = rand() % 5;
				while (a3 == a4) { a4 = rand() % 5; }
				B[a4] = 1;
				int a5 = rand() % 5;
				while (a5 == a3 || a5 == a4) { a5 = rand() % 5; }
				B[a5] = 1;
			}
			else if (a2 == 1) {
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
		{
			unique_lock<mutex> LG1(m11);
			call_once(Open_flag, [&]() {out.open("log.txt"); });
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
			cv1.notify_all();
			out << "Updated Buffer State: " << Buffer;
			out << "Updated Pickup Order: " << update_pickup;
			out << "Local State: " << local_state;
			out << "Updated Cart State: " << product_cart;
			bool flag = false;
			bool dummy = true;
			system_clock::time_point t11{ system_clock::now() };
			if (vectorSum(update_pickup) > 0) {
				//out << "Total Completed Products:" << Products << endl << endl;
				while (cv2.wait_for(LG1, chrono::microseconds(MaxTimeProduct)) != cv_status::timeout) {
					//if condition needed to check if change is possible
					flag = false;
					if (transfer_check(update_pickup)) {
						system_clock::time_point t4{ system_clock::now() };
						system_clock::duration Delay3{ t4 - t1 };
						system_clock::duration Delay1{ t4 - t11 };
						out << "Current Time: " << chrono::duration_cast<microseconds>(Delay3).count() << " us" << endl;
						//out << "Product thread wakes up" << endl;
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
						out << "Total Completed Products: " << Products << endl << endl;
					}
					if (vectorSum(update_pickup) == 0) { //break from the while loop
						flag = true; 
						break; 
					}
				}
			}
			else if(vectorSum(update_pickup)==0) {
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
				out << "Updated Cart State: " << "( 0,0,0,0,0 )" << endl;
				out << "Total Completed Products: " << Products << endl << endl;
				out << "Product Order Complete" << endl; 
			}
			cv1.notify_all();
			if (flag && dummy) { 
				system_clock::time_point t5{ system_clock::now() };
				system_clock::duration Delay5{ t5 - t1 };
				system_clock::duration Delay6{ t5 - t11 };
				out << "Current Time: " << chrono::duration_cast<microseconds>(Delay5).count() << " us" << endl;
				//out << "Product thread wakes up" << endl;
				out << "Iteration " << (i1 + 1) << endl;
				out << "Product Worker ID: " << p << endl;
				out << "Status: Wakeup-Notified" << endl;
				out << "Accumulated Wait Time: " << chrono::duration_cast<microseconds>(Delay6).count() << "us" << endl;
				out << "Buffer State: " << Buffer;
				out << "Pickup Order: " << update_pickup;
				out << "Local State: " << local_state;
				out << "Cart State: " << product_cart;
				out << "Updated Buffer State: " << Buffer;
				out << "Updated Pickup Order: " << update_pickup;
				out << "Local State: " << local_state;
				out << "Cart State: " << product_cart;
				int j = product_cart[0] * 200 + product_cart[1] * 200 + product_cart[2] * 300 + product_cart[3] * 300 + product_cart[4] * 400;
				this_thread::sleep_for(chrono::microseconds(j));
				int j1 = product_cart[0] * 600 + product_cart[1] * 600 + product_cart[2] * 700 + product_cart[3] * 700 + product_cart[4] * 800;
				int j2 = local_use[0] * 600 + local_use[1] * 600 + local_use[2] * 700 + local_use[3] * 700 + local_use[4] * 800;
				this_thread::sleep_for(chrono::microseconds(j1 + j2));
				Products++;
				system_clock::time_point t3{ system_clock::now() };
				system_clock::duration Delay2{ t3 - t1 };
				for (int i = 0; i < 5; i++) {
					local_state[i] -= local_use[i];
				}
				out << "Current Time: " << chrono::duration_cast<microseconds>(Delay2).count() << " us" << endl;
				out << "Updated Local State: " << local_state << endl;
				out << "Updated Cart State: " << "( 0,0,0,0,0 )" << endl;
				out << "Total Completed Products: " << Products << endl << endl;
			}
			else if (!flag && dummy) { 
				//continue;
				system_clock::time_point t2{ system_clock::now() };
				system_clock::duration Delay7{ t2 - t1 }; 
				system_clock::duration Delay6{ t2 - t11 };
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
				out << "Cart State: " << product_cart;
				int j = product_cart[0] * 200 + product_cart[1] * 200 + product_cart[2] * 300 + product_cart[3] * 300 + product_cart[4] * 400;
				this_thread::sleep_for(chrono::microseconds(j));
				if (vectorSum(update_pickup) != 0) {
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
				else if(vectorSum(update_pickup)==0) {
					
					int j1 = product_cart[0] * 600 + product_cart[1] * 600 + product_cart[2] * 700 + product_cart[3] * 700 + product_cart[4] * 800;
					int j2 = local_use[0] * 600 + local_use[1] * 600 + local_use[2] * 700 + local_use[3] * 700 + local_use[4] * 800;
					this_thread::sleep_for(chrono::microseconds(j1 + j2));
					Products++;
					system_clock::time_point t23{ system_clock::now() };
					system_clock::duration Delay8{ t23 - t1 };
					out << "Current Time: " << chrono::duration_cast<microseconds>(Delay8).count() << " us" << endl;
					for (int i = 0; i < 5; i++) {
						local_state[i] -= local_use[i];
						product_cart[i] = 0;
					}
					out << "Updated Local State: " << local_state;
					out << "Updated Cart State: " << product_cart;
					out << "Total Completed Products: " << Products << endl << endl;
				}
			}
			if(dummy) cv1.notify_all();
		}
	}
}
