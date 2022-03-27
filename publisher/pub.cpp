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
    int iterations = 5;
    while(count < iterations){
        sleep(5);
        thePublisher.publishAll();
	count++;
    }

    thePublisher.disconnect();
}
