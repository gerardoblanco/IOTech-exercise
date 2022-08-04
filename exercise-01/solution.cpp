#include <iostream>
#include <string> 
#include <vector>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include <fstream>
#include <string>
#include <algorithm>

using namespace std;


// function to find the UUID of a device by finding the index of the first
// UUID character (preceded by 'uuid:') and the last one (succeeded by a ',')
string uuidFinder(string str){

	int uuidIndexStart = str.find("uuid:") + 5;
	int uuidIndexEnd = str.find(',', uuidIndexStart);

	string uuid = str.substr(uuidIndexStart, uuidIndexEnd-uuidIndexStart);

	return uuid;
}


/* 
	function to add the payloads of a given JSON object and store the sum
	in a vector
*/
void payloadAdder(Json::Value values, vector<int>& vec, int index){
	// declare variables to hold the two sensor payloads
	int p1 = values[index]["Sensors"][0]["Payload"].asInt();
	int p2 = values[index]["Sensors"][1]["Payload"].asInt();

	vec.push_back(p1+p2);
}

/*
	function used to overload 'sort()' using a custom comparator function
	as the third parameter. This way, I can account for device names that start
	with a lower-case letter and would be, by default, placed after all the 
	upper-case-starting device names despite the device name having an
	earlier position in the correct alphabetical order.
*/
bool alphComparator(pair<string,int> &left, pair<string,int> &right){

	return (char)toupper(left.first[1]) < (char)toupper(right.first[1]);

}

string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
        }
        return str;
    }

int main()
{

	ifstream file("data/devices.json");
	Json::Value jsonValues;
	//Json::Value data;
	Json::Reader reader;
	Json::StreamWriterBuilder builder;


	reader.parse(file, jsonValues);

	string rootName = jsonValues.begin().name();

	// vector to hold the uuid's of each device
	vector<string> deviceUuid;

	// vector to hold the device name, and the original index (position) of the device in 'devices.json'
	vector< pair <string,int> > deviceNames;

	// vector to hold the sum of each device payload
	vector<int> payloadSum;

	for (int i = 0; i < jsonValues[rootName].size(); i++)
	{
		string foundUuid = uuidFinder(jsonValues[rootName][i]["Info"].asString());
		deviceUuid.push_back(foundUuid);

		payloadAdder(jsonValues[rootName],payloadSum, i);

		deviceNames.push_back(make_pair(jsonValues[rootName][i]["Name"].asString(), i));
	}

	sort(deviceNames.begin(), deviceNames.end(), alphComparator);
	
	Json::Value root;

	int it = 0;
	for (const auto& pair : deviceNames)
	{	
		Json::Value device;
		device["*1*Name"] = pair.first;
		device["*2*Type"] = jsonValues[rootName][pair.second]["Type"].asString();
		device["*3*Info"] = jsonValues[rootName][pair.second]["Info"].asString();;
		device["*4*UUID"] = deviceUuid[pair.second];
		device["*5*PayloadTotal"] = payloadSum[pair.second];

		string concat = "*"+ to_string(it+1) +"*Device "+ to_string(it+1);

		root[concat]=device;
		it += 1;
	}

	Json::StyledWriter writer;
	string resultString=writer.write(root);

	resultString=ReplaceAll(resultString,"*1*", "");
	resultString=ReplaceAll(resultString,"*2*", "");
	resultString=ReplaceAll(resultString,"*3*", "");
	resultString=ReplaceAll(resultString,"*4*", "");
	resultString=ReplaceAll(resultString,"*5*", "");

	cout<< resultString <<endl;

	ofstream myfile;
	myfile.open ("output.json");
	myfile << resultString;
	myfile.close();
	

	return 0;
}

