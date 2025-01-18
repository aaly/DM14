#include "M14Helper.hpp"

int setArgs(int argc, char *argv[], Node &n, string &mainParameters) {
  for (int i = 1; i < argc; i++) {
    int switchType = 0;
    string S = argv[i];

    if (S == "-P") {
      switchType = 1;
    } else if (S == "-S") {
      switchType = 2;
    } else if (S == "-N") {
      switchType = 3;
    }

    if (switchType == 1 || switchType == 2) {
      i++;
      int port = 0;
      string ip;
      ip = argv[i];
      stringstream SS;
      SS << ip.substr(ip.find(':') + 1, ip.size());
      SS >> port;
      ip = ip.substr(0, ip.find(':'));

      if (switchType == 1) {
        cout << "PIP:" << ip << " PORT:" << port << endl;
        n.addNode(ip, port, true);
      } else if (switchType == 2) {
        cout << "SIP:" << ip << " PORT:" << port << endl;
        n.setServer(ip, port, 60);
      }
    } else if (switchType == 3) {
      i++;
      int nodeN = 0;
      stringstream SS;
      SS << string(argv[i]);
      SS >> nodeN;
      n.setNodeNumber(nodeN);
    } else {
      mainParameters += S;
      if (i + 1 < argc) {
        mainParameters += " ";
      }
    }
  }
  return 0;
};
