#include "machine/mmu.hh"

// GUIDIOS: Agregar coremap al contexto global

class CoreMap{
private:
    Bitmap  *pageMap;
    AddressSpace  **ownerAddSpc;
    unsigned int *virtualPageNum;
    // GUIDIOS:Plus possibly other flags, such as whether the page is
    // currently locked in memory for I/O purposes.

    // GUIDIOS: Nismanear y usar algo mejor el proximo ejercicio :)
    unsigned int nextNisman;

public:
    CoreMap();

    ~CoreMap();

    unsigned int ReservePage(unsigned int virtualPage);
};
