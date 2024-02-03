
#include "libprimis.h"
#include "testutils.h"
#include "testidents.h"
#include "testcs.h"
#include "testshadowatlas.h"
#include "testprops.h"
#include "testocta.h"
#include "testgltfloader.h"
#include "testskel.h"

int main()
{
    testutils();

    testidents();
    testcs();
    testpacknode();
    test_props();
    test_octa();
    test_gltf();
    test_skel();
    return EXIT_SUCCESS;
}
