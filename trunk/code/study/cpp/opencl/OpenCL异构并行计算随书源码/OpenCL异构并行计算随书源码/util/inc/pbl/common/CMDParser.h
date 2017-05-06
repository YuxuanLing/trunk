/**
 * @file
 * @author yyfn
 * @date 20120723
 * 
 * @brief using for parse cmd string likes
 * gcc x d=2
 * x is used for identify something
 * d=2 is keyvalue, value just support int and double/float
 * 
 */

#pragma once
#include <string>
#include <vector>
#include <map>
#include "stringUtil.h"

class CMDParser {
private:
	std::vector<std::string> bcmd; //used for bool cmd
	std::map<std::string, std::string> kv; //used for key=value
public:
	CMDParser(std::string cmd, std::string delim) {
		std::vector < std::string > v;
		split(cmd, delim, &v);

		size_t len = v.size();
		for (size_t i = 0; i < len; i++) {
			std::vector < std::string > sv;
			split(v[i], "=", &sv);
			if (1 == sv.size()) {
				bcmd.push_back(sv[0]);
			} else if (2 == sv.size()) {
				kv.insert(make_pair(sv[0], sv[1]));
			} else {
				printf("%s is not good cmd, omit it\n", v[i].c_str());
			}
		}
	}
	/**
	 * @brief test whether has a bool identify or not
	 */
	bool contain(std::string cmd) {
		size_t len = bcmd.size();
		for (size_t i = 0; i < len; i++) {
			if (cmd == bcmd[i]) {
				return true;
			}
		}

		return false;
	}

	float getFloat(std::string cmd) {

		std::map<std::string, std::string>::iterator iter = kv.find(cmd);
		if (kv.end() != iter) {
			std::string r = iter->second;
			return (float) Atof(r.c_str());
		} else {
			printf("There is no %s, using 0 \n", cmd.c_str());
			return 0.0f;
		}
	}

	int getInt(std::string cmd) {

		std::map<std::string, std::string>::iterator iter = kv.find(cmd);
		if (kv.end() != iter) {
			std::string r = iter->second;
			return Atoi(r.c_str());
		} else {
			printf("There is no %s, using 0 \n", cmd.c_str());
			return 0;
		}
	}

};
