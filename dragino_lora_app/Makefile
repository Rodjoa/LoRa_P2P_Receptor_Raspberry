# Dragino LoRa App - Raspberry Pi
# Compila en C++ y enlaza wiringPi y MQTT

CC = g++
CFLAGS = -c -Wall -std=c++11
LIBS = -lwiringPi -lpaho-mqtt3c

# Nombre del binario
TARGET = dragino_lora_app
SRC = main.cpp
OBJ = main.o

# Regla por defecto
all: $(TARGET)

# Compilar el binario
$(TARGET): $(OBJ)
	@echo "🔨 Linking $(TARGET)..."
	$(CC) $(OBJ) $(LIBS) -o $(TARGET)
	@echo "✅ $(TARGET) creado!"

# Compilar objeto
$(OBJ): $(SRC)
	@echo "🔨 Compiling $(SRC)..."
	$(CC) $(CFLAGS) $(SRC)

# Limpiar archivos compilados
clean:
	@echo "🧹 Limpiando archivos compilados..."
	rm -f $(OBJ) $(TARGET)
	@echo "✅ Limpieza completada!"

# Fuerza recompilación completa
rebuild: clean all

