#include "Barcode.h"

Barcode::Barcode() { voices = nullptr; }

void Barcode::init(Voice *voices) { this->voices = voices; }
