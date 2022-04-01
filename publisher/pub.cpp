#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include "publish.h"


using namespace std;
using namespace exploringRPi;
using namespace ee513a2;

int main(int argc, char* argv[]) {

    Publish thePublisher;

    thePublisher.publishAll();

    int count = 0;
    int iterations = 10;
    while(count < iterations){
        sleep(8);
        thePublisher.publishAll();
	count++;
    }

    thePublisher.disconnect();
}
