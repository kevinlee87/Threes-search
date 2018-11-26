#pragma once
#include <iostream>
#include <algorithm>
#include <cmath>
#include "board.h"
#include <numeric>

class state_type {
public:
	enum type : char {
		before  = 'b',
		after   = 'a',
		illegal = 'i'
	};

public:
	state_type() : t(illegal) {}
	state_type(const state_type& st) = default;
	state_type(state_type::type code) : t(code) {}

	friend std::istream& operator >>(std::istream& in, state_type& type) {
		std::string s;
		if (in >> s) type.t = static_cast<state_type::type>((s + " ").front());
		return in;
	}

	friend std::ostream& operator <<(std::ostream& out, const state_type& type) {
		return out << char(type.t);
	}

	bool is_before()  const { return t == before; }
	bool is_after()   const { return t == after; }
	bool is_illegal() const { return t == illegal; }

private:
	type t;
};

class state_hint {
public:
	state_hint(const board& state) : state(const_cast<board&>(state)) {}

	char type() const { return state.info() ? state.info() + '0' : 'x'; }
	operator board::cell() const { return state.info(); }

public:
	friend std::istream& operator >>(std::istream& in, state_hint& hint) {
		while (in.peek() != '+' && in.good()) in.ignore(1);
		char v; in.ignore(1) >> v;
		hint.state.info(v != 'x' ? v - '0' : 0);
		return in;
	}
	friend std::ostream& operator <<(std::ostream& out, const state_hint& hint) {
		return out << "+" << hint.type();
	}

private:
	board& state;
};

class solver {
public:
	typedef float value_t;

public:
	class answer {
	public:
		answer(value_t min = 0.0/0.0, value_t avg = 0.0/0.0, value_t max = 0.0/0.0) : min(min), avg(avg), max(max) {}
	    friend std::ostream& operator <<(std::ostream& out, const answer& ans) {
	    	return !std::isnan(ans.avg) ? (out << ans.min << " " << ans.avg << " " << ans.max) : (out << "-1") << std::endl;
		}
	public:
		value_t min, avg, max;
	};

public:
	solver(const std::string& args) {
		reset();
		int tile[6] = {0,0,0,0,0,0};
		table = new answer[100000000];
		dfs('a',tile,1,-1);
		dfs('a',tile,2,-1);
		dfs('a',tile,3,-1);
	}

	answer solve(const board& state, state_type type = state_type::before) {
		// TODO: find the answer in the lookup table and return it
		//       do NOT recalculate the tree at here
		// to fetch the hint (if type == state_type::after, hint will be 0)
		int i, current, key, skip;
		int tile[6];
		float min, avg, max;
		skip = 0;
		index[1] = 0;
		index[2] = 1;
		current = 3;
		for(i = 3;i <= 128;i = i * 2){
			index[i] = current;
			current++;
		}
		for(i = 0;i < 6;i++){
			tile[i] = state(i);
		}
		board::cell hint = state_hint(state);
		if(skip == 0){
			if(type.is_after()){
				key = tile[0] * 10000000 + tile[1] * 1000000 + tile[2] * 100000 + tile[3] * 10000 + tile[4] * 1000 + tile[5] * 100 + hint * 10;
				if(!std::isnan(table[key].avg)){
					min = table[key].min;
					avg = table[key].avg;
					max = table[key].max;
					return { min, avg, max };
				}
				return {-1};
			}
			else if(type.is_before()){
				key = 9 * 10000000 + tile[0] * 1000000 + tile[1] * 100000 + tile[2] * 10000 + tile[3] * 1000 + tile[4] * 100 + tile[5] * 10 + hint;
				if(!std::isnan(table[key].avg)){
					min = table[key].min;
					avg = table[key].avg;
					max = table[key].max;
					return { min, avg, max };
				}
				return {-1};
			}
		}
		return {-1};
	}
	
	void dfs(char type,int tile[6], int hint, int pre_action){
		int move, move_flag, cur, key, s;
		int save[6];
		float sum, max, avg_s;
		//std::cout << type << "\n" << tile[0] << " " << tile[1] << " " << tile[2] << "   +" << hint << "\n" << tile[3] << " " << tile[4] << " " << tile[5] << "   " << pre_action << "\n";
		if(pre_action == -1)	return;
		else{
			for(int i = 0;i < 6;i++)	save[i] = tile[i];
			if(type == 'a'){
				cur = 0;
				if(bag[0] == 0 && bag[1] == 0 && bag[2] == 0)	reset();
				if(pre_action == -1){
					for(int pos = 0;pos < 6;pos++){
						tile[pos] = hint;
						bag[hint - 1] = 0;
						dfs('b',tile,hint,pre_action);
						key = tile[0] * 10000000 + tile[1] * 1000000 + tile[2] * 100000 + tile[3] * 10000 + tile[4] * 1000 + tile[5] * 100 + hint * 10;
						avg_s = table[key].avg;
						if(max < avg_s)	s = key;
						sum += avg_s;
						cur++;
						bag[hint - 1] = hint;
					}
					table[hint * 10].min = table[s].min;
					table[hint * 10].avg = sum / (float)cur;
					table[hint * 10].max = table[s].max;
				}
				else{
					sum = 0;
					switch(pre_action){
						case 0:		//up
							for(int pos = 3;pos < 6;pos++){
								if(tile[pos] == 0){
									tile[pos] = hint;
									for(int i = 0;i < 3;i++){
										if(bag[i] != 0){
											hint = bag[i];
											bag[i] = 0;
											dfs('b',tile,hint,pre_action);
											key = 9 * 10000000 + tile[0] * 1000000 + tile[1] * 100000 + tile[2] * 10000 + tile[3] * 1000 + tile[4] * 100 + tile[5] * 10 + hint;
											avg_s = table[key].avg;
											if(max < avg_s)	s = key;
											sum += avg_s;
											cur++;
											bag[i] = i + 1;
										}
									}
									key = tile[0] * 10000000 + tile[1] * 1000000 + tile[2] * 100000 + tile[3] * 10000 + tile[4] * 1000 + tile[5] * 100 + hint * 10 + pre_action;
									table[key].min = table[s].min;
									table[key].avg = sum / (float)cur;
									table[key].max = table[s].max;
								}
							}
							break;
						case 1:		//right
							for(int pos = 0;pos <= 3;pos = pos + 3){
								if(tile[pos] == 0){
									tile[pos] = hint;
									for(int i = 0;i < 3;i++){
										if(bag[i] != 0){
											hint = bag[i];
											bag[i] = 0;
											dfs('b',tile,hint,pre_action);
											key = 9 * 10000000 + tile[0] * 1000000 + tile[1] * 100000 + tile[2] * 10000 + tile[3] * 1000 + tile[4] * 100 + tile[5] * 10 + hint;
											avg_s = table[key].avg;
											if(max < avg_s)	s = key;
											sum += avg_s;
											cur++;
											bag[i] = i + 1;
										}
									}
									key = tile[0] * 10000000 + tile[1] * 1000000 + tile[2] * 100000 + tile[3] * 10000 + tile[4] * 1000 + tile[5] * 100 + hint * 10 + pre_action;
									table[key].min = table[s].min;
									table[key].avg = sum / (float)cur;
									table[key].max = table[s].max;
								}
							}
							break;
						case 2:		//down
							for(int pos = 0;pos < 3;pos++){
								if(tile[pos] == 0){
									tile[pos] = hint;
									for(int i = 0;i < 3;i++){
										if(bag[i] != 0){
											hint = bag[i];
											bag[i] = 0;
											dfs('b',tile,hint,pre_action);
											key = 9 * 10000000 + tile[0] * 1000000 + tile[1] * 100000 + tile[2] * 10000 + tile[3] * 1000 + tile[4] * 100 + tile[5] * 10 + hint;
											avg_s = table[key].avg;
											if(max < avg_s)	s = key;
											sum += avg_s;
											cur++;
											bag[i] = i + 1;
										}
									}
									key = tile[0] * 10000000 + tile[1] * 1000000 + tile[2] * 100000 + tile[3] * 10000 + tile[4] * 1000 + tile[5] * 100 + hint * 10 + pre_action;
									table[key].min = table[s].min;
									table[key].avg = sum / (float)cur;
									table[key].max = table[s].max;
								}
							}
							break;
						case 3:		//left
							for(int pos = 2;pos <= 5;pos = pos + 3){
								if(tile[pos] == 0){
									tile[pos] = hint;
									for(int i = 0;i < 3;i++){
										if(bag[i] != 0){
											hint = bag[i];
											bag[i] = 0;
											dfs('b',tile,hint,pre_action);
											key = 9 * 10000000 + tile[0] * 1000000 + tile[1] * 100000 + tile[2] * 10000 + tile[3] * 1000 + tile[4] * 100 + tile[5] * 10 + hint;
											avg_s = table[key].avg;
											if(max < avg_s)	s = key;
											sum += avg_s;
											cur++;
											bag[i] = i + 1;
										}
									}
									key = tile[0] * 10000000 + tile[1] * 1000000 + tile[2] * 100000 + tile[3] * 10000 + tile[4] * 1000 + tile[5] * 100 + hint * 10 + pre_action;
									table[key].min = table[s].min;
									table[key].avg = sum / (float)cur;
									table[key].max = table[s].max;
								}
							}
							break;
					}
				}
			}
			else if(type == 'b'){
				move = 0;
				move_flag = 0;
				s = 9 * 10000000 + tile[0] * 1000000 + tile[1] * 100000 + tile[2] * 10000 + tile[3] * 1000 + tile[4] * 100 + tile[5] * 10 + hint;
				for(int i = 0;i < 3;i++){
					if(tile[i + 3] == 0)	continue;
					if(tile[i] == 0){
						tile[i] = tile[i + 3];
						tile[i + 3] = 0;
						move = 1;
					}
					else if(tile[i] == tile[i + 3] && tile[i] > 2){
						tile[i]++;
						tile[i + 3] = 0;
						move = 1;
					}
					else if((tile[i] + tile[i + 3]) == 3){
						tile[i] = 3;
						tile[i + 3] = 0;
						move = 1;
					}
				}
				if(move == 1){
					dfs('a',tile,hint,0);
					key = tile[0] * 10000000 + tile[1] * 1000000 + tile[2] * 100000 + tile[3] * 10000 + tile[4] * 1000 + tile[5] * 100 + hint * 10;
					table[s].min = table[key].min;
					table[s].avg = table[key].avg;
					table[s].max = table[key].max;
					move_flag = 1;
				}
				for(int i = 0;i < 6;i++)	tile[i] = save[i];
				move = 0;
				for(int i = 0;i < 2;i++){
					for(int j = 2;j > 0;j--){
						if(tile[i * 3 + j - 1] == 0)	continue;
						if(tile[i * 3 + j] == 0){
							tile[i * 3 + j] = tile[i * 3 + j - 1];
							tile[i * 3 + j - 1] = 0;
							move = 1;
						}
						else if(tile[i * 3 + j] == tile[i * 3 + j - 1] && tile[i * 3 + j] > 2){
							tile[i * 3 + j]++;
							tile[i * 3 + j - 1] = 0;
							move = 1;
						}
						else if((tile[i * 3 + j] + tile[i * 3 + j - 1]) == 3){
							tile[i * 3 + j] = 3;
							tile[i * 3 + j -1] = 0;
							move = 1;
						}
					}
				}
				if(move == 1){
					dfs('a',tile,hint,1);
					key = tile[0] * 10000000 + tile[1] * 1000000 + tile[2] * 100000 + tile[3] * 10000 + tile[4] * 1000 + tile[5] * 100 + hint * 10 + 1;
					table[s].min = table[key].min;
					table[s].avg = table[key].avg;
					table[s].max = table[key].max;
					move_flag = 1;
				}
				for(int i = 0;i < 6;i++)	tile[i] = save[i];
				move = 0;
				for(int i = 0;i < 3;i++){
					if(tile[i] == 0)	continue;
					if(tile[i + 3] == 0){
						tile[i + 3] = tile[i];
						tile[i] = 0;
						move = 1;
					}
					else if(tile[i] == tile[i + 3] && tile[i] > 2){
						tile[i + 3]++;
						tile[i] = 0;
						move = 1;
					}
					else if((tile[i] + tile[i + 3]) == 3){
						tile[i + 3] = 3;
						tile[i] = 0;
						move = 1;
					}
				}
				if(move == 1){
					dfs('a',tile,hint,2);
					key = tile[0] * 10000000 + tile[1] * 1000000 + tile[2] * 100000 + tile[3] * 10000 + tile[4] * 1000 + tile[5] * 100 + hint * 10 + 2;
					table[s].min = table[key].min;
					table[s].avg = table[key].avg;
					table[s].max = table[key].max;
					move_flag = 1;
				}
				
				for(int i = 0;i < 6;i++)	tile[i] = save[i];
				move = 0;
				for(int i = 0;i < 2;i++){
					for(int j = 0;j < 2;j++){
						if(tile[i * 3 + j + 1] == 0)	continue;
						if(tile[i * 3 + j] == 0){
							tile[i * 3 + j] = tile[i * 3 + j + 1];
							tile[i * 3 + j + 1] = 0;
							move = 1;
						}
						else if(tile[i * 3 + j] == tile[i * 3 + j + 1] && tile[i * 3 + j] > 2){
							tile[i * 3 + j]++;
							tile[i * 3 + j + 1] = 0;
							move = 1;
						}
						else if((tile[i * 3 + j] + tile[i * 3 + j + 1]) == 3){
							tile[i * 3 + j] = 3;
							tile[i * 3 + j + 1] = 0;
							move = 1;
						}
					}
				}
				if(move == 1){
					dfs('a',tile,hint,3);
					key = tile[0] * 10000000 + tile[1] * 1000000 + tile[2] * 100000 + tile[3] * 10000 + tile[4] * 1000 + tile[5] * 100 + hint * 10 + 3;
					table[s].min = table[key].min;
					table[s].avg = table[key].avg;
					table[s].max = table[key].max;
					move_flag = 1;
				}
				if(move_flag == 0){
					sum = 0;
					for(int i = 0;i < 6;i++){
						if(tile[i] > 2)	sum += pow(3,tile[i] - 2);
					}
					table[s].min = sum;
					table[s].avg = sum;
					table[s].max = sum;
				}
			}
		}
	}
	
	void reset(){
		bag = {1, 2, 3};
	}

private:
	answer* table;
	int index[1000];
	std::array<int, 3> bag;
};


