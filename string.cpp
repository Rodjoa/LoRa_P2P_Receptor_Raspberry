#include <string>
#include <iostream>
int main() {
    std::string payload = "Ijj,Timestamp, Lat";
    //Borra hasta encontrar la primera coma "," y borra también la coma ","
    int i = 0;
    while (payload[i] != ',') { //usar comillas simples para que ambos sean char
      payload.erase(i, 1); // Removes 1 character starting at index 0 (the space)
      i;//No se incrementa porque al borrar el 1er char, el 2do pasa a ser 1ero
    }
    payload.erase(i, 1); // Removes 1 character starting at index 0 (the space)
    // str is now "HelloWorld"
    std::cout << "payload: " << payload << std::endl; 

    return 0;
}