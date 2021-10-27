//#define _SILENCE_EXPERIMENTAL_FILESYSTER_DEPRECATION_WARNING

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#include <fstream>
#include <experimental/filesystem>
#include <locale>

#pragma warning(disable: 4996)



using namespace std;
SOCKET Connection;

string IP = "127.0.0.1";

string Path = "Data.json", text;

// Проверка на вход
bool acces = false;

// Конвертация в Json
vector <pair <string, string> > Parsing_Json_In(string adress)
{
	string str, name, value;
	int k;
	bool key1 = false, key2 = false, key3 = false;
	vector < pair < string, string> > data;
	ifstream in(adress);
	while (str[0] != '}' )
	{
		k = 0;
		name = "";
		value = "";
		key1 = false, key2 = false, key3 = false;
		getline(in, str);
		for (int i = 0; i != str.size(); i++)
		{
			if (str[i] == '"') { k++; }
			if (k % 2 == 1)key1 = true; else key1 = false;
			if (str[i] == ':')
			{
				key3 = true;
				if (str[i + 1] != '"')key2 = true;
			}
			if (str[i] == ',')break;
			if (!key3)
			{
				if (key1 == true) {
					if (str[i] != '"')
						name += str[i];
				}
			}
			else
			{
				if (key2 == true) {
					if (str[i] != '<' && str[i] != '>' && str[i] != '"' && str[i] != ':')
						value += str[i];
				}
				if (key1 == true) {

					if (str[i] != '<' && str[i] != '>' && str[i] != '"' && str[i] != ':')
						value += str[i];
				}
			}
		}

		if (!(name.size() == 0 || value.size() == 0))
		{
			data.push_back(make_pair(name, value));
		}

	}
	in.close();
	return data;
}

// Парсинг из Json
void Parsing_Json_Out(vector <pair <string, string> > data, string adress)
{

	ofstream  out(adress);
	out << "{" << endl;
	bool key;
	for (int i = 0; i != data.size(); i++)
	{
		out << " ";
		if (data[i].first == "id")
		{
			out << '"' << data[i].first << '"' << ':' << data[i].second;
		}
		else
		{
			out << '"' << data[i].first << '"' << ':' << '"' << data[i].second << '"';
		}
		out << "," << endl;
	}
	out << "}";
}

// Отправка файла Json
void send_file(SOCKET* sock, const string& file_name) 
	{
		fstream file;
		file.open(file_name, ios_base::in | ios_base::binary);
		if (file.is_open()) {
			int file_size = experimental::filesystem::file_size(file_name) +1 ;

			char* bytes = new char[file_size];
			file.read((char*)bytes, file_size);
	

			send(*sock, to_string(file_size).c_str(), 16, 0);
			send(*sock, file_name.c_str(), 32, 0);
			send(*sock, bytes, file_size, 0);


		}
		else {
			cout << "ERORR With files" << endl;
		}
		file.close();

	}

// Принятие файла Json
void recv_file(SOCKET* sock)
{
	char file_size_str[16];
	char file_name[32];
	recv(*sock, file_size_str, 16, 0);
	int file_size = atoi(file_size_str);
	char* bytes = new char[file_size];

	recv(*sock, file_name, 32, 0);
	fstream file;
	file.open(file_name, ios_base::out | ios_base::binary);

		if (file.is_open()) {
			recv(*sock, bytes, file_size, 0);
			file.write(bytes, file_size);
		}
		else
		{
			cout << "Error open";
		}
		delete[] bytes;
		file.close();
}

// Принятие клиента
void ClientHandler() {

	while (true) {
		recv_file(&Connection);
		if (acces == true)
		{
			
			vector < pair <string, string > > data;
			data = Parsing_Json_In(Path);
			if (data.size() != 0) {
				if (data.size() > 2) {
					if (data[0].second == "message_reply")
						if (data[1].second == "failed") {
							cout << data[2].second << endl;
						}
				}
				if (data.size() > 2) {
					if (data[0].second == "message")
					{

						cout << data[2].second << ": " << data[1].second << endl;
					}
				}
			}
		}
	}
}

// Отправление клиента
int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "Russian");
	//WSAStartup
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	Connection = socket(AF_INET, SOCK_STREAM, NULL);
	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
		std::cout << "Error: failed connect to server.\n";
		return 1;
	}


	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

	vector < pair <string, string > > data;
	
	data.push_back(make_pair("command", "HELLO"));
	Parsing_Json_Out(data, Path);
	data.clear();  Sleep(30);
	send_file(&Connection, Path);

	data = Parsing_Json_In(Path);
	Sleep(200);
	if (data.size() == 1) {
		if (data[0].first == "command")
		{
			if (data[0].second == "HELLO")
			{
				cout << "Connected"<<endl;
			}
		}
	}

	string login;
	string password;

	bool Accept = true;
	while (Accept) {
         
		data.clear();
		string command = "";
		while (true) {

			cout << "Write: Login or Registr:" << endl;
			cin >> command;
			if (command == "Login" || command == "Registr")break;
			cout << "Uncorrect" << endl;
		}

		string nickname;

		if (command == "Registr")
		{
			cout << "Login:" << endl;
			cin >> login;
			cout << "Password:" << endl;
			cin >> password;
			cout << "Nickname:" << endl;
			cin >> nickname;
			data.clear();
			data.push_back(make_pair("command", "register"));
			data.push_back(make_pair("login", login));
			data.push_back(make_pair("password", password));
			data.push_back(make_pair("nickname", nickname));

			Parsing_Json_Out(data, Path);
			data.clear();  Sleep(30);
			send_file(&Connection, Path);

			Sleep(100);
			data = Parsing_Json_In(Path);
			
			if (data[0].second == "register_reply")
			{
				if (data[1].first == "status") cout << data[2].second << endl;
			}
		}
		else
		{
			cout << "Login:" << endl;
			cin >> login;
			cout << "Password:" << endl;
			cin >> password;
			data.push_back(make_pair("command", "login"));
			data.push_back(make_pair("login", login));
			data.push_back(make_pair("password", password));


			Parsing_Json_Out(data, Path);
			data.clear();  Sleep(30);
			send_file(&Connection, Path);

			Sleep(100);

			data = Parsing_Json_In(Path);

			if (data.size() != 3)
			{
				cout << "Error" << endl;
			}
			else {
				if (data[0].first == "command" && data[0].second == "login")
				{
					if (data[1].first == "status" && data[1].second == "ok")
					{
						if (data[2].first == "session" && data[2].second == IP)
						{
							system("cls");
							cout << "Accepted" << endl;
							Sleep(1500);
							system("cls");
							Accept = false;
							acces = true;
							break;
						}
					}
				}

			}
			if (data[data.size() - 1].first == "message")
			{
				cout << data[data.size() - 1].second << endl;
			}
		}
	}
	string message;

	getline(cin, message);
	while (true){

		getline(cin, message);
	// Парсинг Json In
		data.clear();
		data.push_back(make_pair("command", "message"));
		data.push_back(make_pair("body", message));
		data.push_back(make_pair("session", IP));
		Parsing_Json_Out(data, Path);
		Sleep(30); data.clear();
        send_file(&Connection, Path);
	}

	system("pause");
	return 0;
}
