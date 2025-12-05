# Makefile for TWM (Tile Window Manager)

# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -O2 -g
INCLUDES = -Isrc

# Библиотеки
# Требуется установка X11 development libraries:
#   Ubuntu/Debian: sudo apt-get install libx11-dev
#   Arch: sudo pacman -S libx11
#   Fedora: sudo dnf install libX11-devel
LIBS = -lX11

# Директории
SRCDIR = src
BUILDDIR = build
OBJDIR = $(BUILDDIR)/obj

# Исходные файлы (только main.cpp для основного исполняемого файла)
SOURCES = $(SRCDIR)/main.cpp
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Имя исполняемого файла
TARGET = $(BUILDDIR)/twm

# Правило по умолчанию
all: $(TARGET)

# Создание исполняемого файла
$(TARGET): $(OBJECTS) | $(BUILDDIR)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)
	@echo "Build complete: $(TARGET)"

# Компиляция объектных файлов
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Создание директорий
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

# Очистка
clean:
	rm -rf $(BUILDDIR)
	@echo "Clean complete"

# Пересборка
rebuild: clean all

# Запуск
run: $(TARGET)
	$(TARGET)

# Установка (опционально)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/twm
	@echo "Installed to /usr/local/bin/twm"

# Удаление установки
uninstall:
	rm -f /usr/local/bin/twm
	@echo "Uninstalled from /usr/local/bin/twm"

# Помощь
help:
	@echo "Available targets:"
	@echo "  all      - Build the project (default)"
	@echo "  clean    - Remove build files"
	@echo "  rebuild  - Clean and build"
	@echo "  run      - Build and run"
	@echo "  install  - Install to /usr/local/bin"
	@echo "  uninstall- Remove from /usr/local/bin"
	@echo "  help     - Show this help"

.PHONY: all clean rebuild run install uninstall help

