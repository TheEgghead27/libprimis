
#include "libprimis.h"
#include "../src/shared/geomexts.h"

constexpr float tolerance = 0.001;

void test_matrix3_ctor()
{
    std::printf("testing matrix3 constructor\n");
    //matrix3()
    {
        matrix3 m;
        assert(m.a == vec(0,0,0));
        assert(m.b == vec(0,0,0));
        assert(m.c == vec(0,0,0));
    }
    //matrix3(vec,vec,vec)
    {
        vec a(1,2,3),
            b(4,5,6),
            c(7,8,9);
        matrix3 m(a,b,c);
        assert(m.a == a);
        assert(m.b == b);
        assert(m.c == c);
    }
    //matrix3(quat)
    {
        //test use of normalized quat -> normalized matrix
        quat q(0.5,0.5,0.5,0.5);
        matrix3 m(q);
        assert(m.a.sub(vec(0,1,0)).magnitude() < tolerance);
        assert(m.b.sub(vec(0,0,1)).magnitude() < tolerance);
        assert(m.c.sub(vec(1,0,0)).magnitude() < tolerance);
    }
    {
        //test identity quat -> identity matrix
        quat q(0,0,0,1);
        matrix3 m(q);
        assert(m.a.sub(vec(1,0,0)).magnitude() < tolerance);
        assert(m.b.sub(vec(0,1,0)).magnitude() < tolerance);
        assert(m.c.sub(vec(0,0,1)).magnitude() < tolerance);
    }
    //matrix3(float,vec)
    {
        //test identity generation
        vec axis(1,0,0);
        float angle = 0;
        matrix3 m(angle, axis);
        assert(m.a.sub(vec(1,0,0)).magnitude() < tolerance);
        assert(m.b.sub(vec(0,1,0)).magnitude() < tolerance);
        assert(m.c.sub(vec(0,0,1)).magnitude() < tolerance);
    }
    {
        //test 360 degree rotation about x
        vec axis(1,0,0);
        float angle = 2*M_PI;
        matrix3 m(angle, axis);
        assert(m.a.sub(vec(1,0,0)).magnitude() < tolerance);
        assert(m.b.sub(vec(0,1,0)).magnitude() < tolerance);
        assert(m.c.sub(vec(0,0,1)).magnitude() < tolerance);
    }
    {
        //test 180 degree rotation about x
        vec axis(1,0,0);
        float angle = M_PI;
        matrix3 m(angle, axis);
        assert(m.a.sub(vec(1,0,0)).magnitude() < tolerance);
        assert(m.b.sub(vec(0,-1,0)).magnitude() < tolerance);
        assert(m.c.sub(vec(0,0,-1)).magnitude() < tolerance);
    }
    {
        //test 180 degree rotation about y
        vec axis(0,1,0);
        float angle = M_PI;
        matrix3 m(angle, axis);
        assert(m.a.sub(vec(-1,0,0)).magnitude() < tolerance);
        assert(m.b.sub(vec(0,1,0)).magnitude() < tolerance);
        assert(m.c.sub(vec(0,0,-1)).magnitude() < tolerance);
    }
    //matrix3(matrix4x3)
    {
        matrix4x3 m1(vec(1,2,3), vec(4,5,6), vec(7,8,9), vec(10,11,12));
        matrix3 m2(m1);
        assert(m2.a == vec(1,2,3));
        assert(m2.b == vec(4,5,6));
        assert(m2.c == vec(7,8,9));
    }
    //matrix3(matrix4)
    {
        matrix4 m1(vec4<float>(1,2,3,4),
                   vec4<float>(5,6,7,8),
                   vec4<float>(9,10,11,12),
                   vec4<float>(13,14,15,16));
        matrix3 m2(m1);
        assert(m2.a == vec(1,2,3));
        assert(m2.b == vec(5,6,7));
        assert(m2.c == vec(9,10,11));
    }
}

void test_matrix3_normalize()
{
    std::printf("testing matrix3 normalize\n");
    {
        //test identity -> identity
        matrix3 m;
        m.identity();
        m.normalize();
        assert(m.a.sub(vec(1,0,0)).magnitude() < tolerance);
        assert(m.b.sub(vec(0,1,0)).magnitude() < tolerance);
        assert(m.c.sub(vec(0,0,1)).magnitude() < tolerance);
    }
    {
        //test normal normalization case
        matrix3 m;
        m.a = vec(3,4,0);
        m.b = vec(0,4,3);
        m.c = vec(4,0,3);
        m.normalize();
        assert(m.a.sub(vec(0.6,0.8,0)).magnitude() < tolerance);
        assert(m.b.sub(vec(0,0.8,0.6)).magnitude() < tolerance);
        assert(m.c.sub(vec(0.8,0,0.6)).magnitude() < tolerance);
    }
}
void test_matrix3_identity()
{
    std::printf("testing matrix3 identity\n");
    matrix3 m;
    m.identity();
    assert(m.a == vec(1,0,0));
    assert(m.b == vec(0,1,0));
    assert(m.c == vec(0,0,1));
}

void test_matrix4x3_mul()
{
    std::printf("testing matrix4x3 mul\n");
    {
        matrix4x3 m;
        m.identity();
        m.mul(2);
        assert(m.a == vec(2,0,0));
        assert(m.b == vec(0,2,0));
        assert(m.c == vec(0,0,2));
        assert(m.d == vec(0,0,0));
    }
}

void test_matrix4x3_setscale()
{
    std::printf("testing matrix4x3 setscale\n");
    matrix4x3 output(vec(1,0,0), vec(0,2,0), vec(0,0,3), vec(0,0,0)),
              output2(vec(2,0,0), vec(0,2,0), vec(0,0,2), vec(0,0,0));
    //setscale(float, float, float)
    {
        matrix4x3 m;
        m.setscale(1,2,3);
        assert(m.a == output.a);
        assert(m.b == output.b);
        assert(m.c == output.c);
    }
    //setscale(vec)
    {
        matrix4x3 m;
        m.setscale(vec(1,2,3));
        assert(m.a == output.a);
        assert(m.b == output.b);
        assert(m.c == output.c);
    }
    //setscale(float)
    {
        matrix4x3 m;
        m.setscale(2);
        assert(m.a == output2.a);
        assert(m.b == output2.b);
        assert(m.c == output2.c);
    }
}

void test_matrix4x3_scale()
{
    std::printf("testing matrix4x3 scale\n");
    matrix4x3 output(vec(1,0,0), vec(0,2,0), vec(0,0,3), vec(0,0,0)),
              output2(vec(2,0,0), vec(0,2,0), vec(0,0,2), vec(0,0,0));
    //setscale(float, float, float)
    {
        matrix4x3 m;
        m.identity();
        m.scale(1,2,3);
        assert(m.a == output.a);
        assert(m.b == output.b);
        assert(m.c == output.c);
    }
    //setscale(vec)
    {
        matrix4x3 m;
        m.identity();
        m.scale(vec(1,2,3));
        assert(m.a == output.a);
        assert(m.b == output.b);
        assert(m.c == output.c);
    }
    //setscale(float)
    {
        matrix4x3 m;
        m.identity();
        m.scale(2);
        assert(m.a == output2.a);
        assert(m.b == output2.b);
        assert(m.c == output2.c);
    }
}

void test_matrix4x3_identity()
{
    std::printf("testing matrix4x3 identity\n");
    matrix4x3 m;
    m.identity();
    assert(m.a == vec(1,0,0));
    assert(m.b == vec(0,1,0));
    assert(m.c == vec(0,0,1));
    assert(m.d == vec(0,0,0));
}

void test_matrix4x3_row()
{
    std::printf("testing matrix4x3 rowx/rowy/rowz\n");

    matrix4x3 m;
    m.identity();
    assert(m.rowx() == vec4<float>(1,0,0,0));
    assert(m.rowy() == vec4<float>(0,1,0,0));
    assert(m.rowz() == vec4<float>(0,0,1,0));
}

void test_matrix4x3_settranslate()
{
    std::printf("testing matrix4x3 settranslate\n");
    //settranslate(vec)
    {
        matrix4x3 m;
        m.identity();
        m.settranslation(vec(1,2,3));
        assert(m.d == vec(1,2,3));
    }
    //settranslate(float,float,float)
    {
        matrix4x3 m;
        m.identity();
        m.settranslation(1,2,3);
        assert(m.d == vec(1,2,3));
    }
}

void test_matrix4_identity()
{
    std::printf("testing matrix4 identity\n");
    matrix4 m;
    m.identity();
    assert(m.a == vec4<float>(1,0,0,0));
    assert(m.b == vec4<float>(0,1,0,0));
    assert(m.c == vec4<float>(0,0,1,0));
    assert(m.d == vec4<float>(0,0,0,1));
}

void test_matrix()
{
    std::printf(
"===============================================================\n\
testing matrices\n\
===============================================================\n"
    );

    test_matrix3_ctor();
    test_matrix3_normalize();
    test_matrix3_identity();

    test_matrix4x3_mul();
    test_matrix4x3_setscale();
    test_matrix4x3_scale();
    test_matrix4x3_settranslate();
    test_matrix4x3_identity();
    test_matrix4x3_row();

    test_matrix4_identity();
}
