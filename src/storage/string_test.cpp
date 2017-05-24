#include <iostream>
#include <string>

std::string months[] = { "Jan", "Feb", "Mar", "April", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"};

int main() {
	for (int i = 0; i < 15; i++) {
		std::cout << months[i] << std::endl;
		if (months[i].empty())
			std::cout << "found 0" << std::endl;
	}	
}
