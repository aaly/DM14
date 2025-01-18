#include "main.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

class parametersHandler {
public:
  parametersHandler(int &argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
      this->parameters.push_back(std::string(argv[i]));
    }
  }

  std::string geOptionValue(const std::string &option) {
    std::string result("");

    for (unsigned int i = 0; i < parameters.size(); i++) {
      if (parameters.at(i) == option) {
        if (i + 1 < parameters.size()) {
          result = parameters.at(i + 1);
          parameters.erase(parameters.begin() + i + 1);
          parameters.erase(parameters.begin() + i);
          break;
        }
      }
    }
    return result;
  }

  bool hasParameterOption(const std::string &option) const {
    return std::find(this->parameters.begin(), this->parameters.end(),
                     option) != this->parameters.end();
  }

private:
  std::vector<std::string> parameters;
};

// we need boolean and arrays
int main(int argc, char **argv) {
  parametersHandler pHandler(argc, argv);

  // when  variavle declared , if no value initialized , then just make a
  // pointer to it wait when the operational Statement that assigns it , then
  // initialize it with NEW , and use it as *P :) make a vector with initialzed
  // variables to check if its initialized yet or not :)

  DM14::types::intSymbols();
  string fname;
  std::vector<std::string> includePaths;
  while (pHandler.hasParameterOption("-I") == true) {
    const std::string &includePath = pHandler.geOptionValue("-I");

    if (!includePath.empty()) {
      displayInfo(" adding   ... [" + includePath + "]");
      includePaths.push_back(includePath);
    }
  }

  if (pHandler.hasParameterOption("--sources")) {
    fname = pHandler.geOptionValue("--sources");
    if (fname.empty()) {
      fname = "testsrc.m14";
    }
  } else {
    fname = "testsrc.m14";
  }

  // cout << " Scanning ... [" << fname << "]" << endl;
  displayInfo(" Scanning  ... [" + fname + "]");
  std::shared_ptr<DM14::scanner> scner(new DM14::scanner(fname));
  scner->setShortComment("~~");
  scner->setLongComment("~*", "*~");
  scner->scan();
  // scner->printTokens();
  // exit(1);

  displayInfo(" Parsing   ... [" + fname + "]");
  std::shared_ptr<DM14::Parser> prser(
      new DM14::Parser(scner->getTokens(), fname, false));

  for (uint32_t i = 0; i < includePaths.size(); i++) {
    displayInfo(" adding   ... [" + includePaths.at(i) + "]");
    prser->addIncludePath(includePaths.at(i));
  }
  // prser->printEBNF();
  prser->parse();

  displayInfo(" Compiling  ... [" + fname + "]");
  std::shared_ptr<DM14::compiler> Compiler(
      new DM14::compiler(prser->getMapCodes()));

  for (uint32_t i = 0; i < includePaths.size(); i++) {
    displayInfo(" adding   ... [" + includePaths.at(i) + "]");
    Compiler->addIncludePath(includePaths.at(i));
  }

  Compiler->setVersion(0.01);
  Compiler->setgccPath("");
  Compiler->setcompileStatic(false);
  Compiler->compile();

  return 0;
}
