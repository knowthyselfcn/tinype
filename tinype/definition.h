#ifndef TINYPE_DEFINITION_H
#define TINYPE_DEFINITION_H

#include <math.h>
#include <assert.h>


/*
示例代码，简单起见，都采用双精度浮点数
*/


typedef struct {
    double  x;
    double 	y;
    double 	z;
} Vector;

typedef Vector Point;





inline double vectorLength(Vector* v) { return sqrt(v->x*v->x + v->y*v->y + v->z*v->z); };

inline Vector addVector(Vector* v1, Vector* v2) {
    return{ v1->x + v2->x, v1->y + v2->y, v1->z + v2->z };
}; // 

// 翻转向量， 用ScalarVector(v, -1) 即可

inline Vector differencePoint(Point* p1, Point* p2) {
    return{ p1->x - p2->x, p1->y - p2->y, p1->z - p2->z };
};

// 某点经过向量的transform，到达新的点，其实应该起名 transformPoint(Point, Vector)
inline Point addPoint(Point* p1, Vector* v) {
    return{ p1->x + v->x, p1->y + v->y, p1->z + v->z };
};

// dotProduct
inline double scalarProduct(Vector* v1, Vector* v2){
    return v1->x*v2->x + v1->y*v2->y + v1->z*v2->z;
};

// 需要注意v1 为x， v2为y，顺序不能改变， 否则构造出来 -z   x * y =z
// z * x = y
inline Vector crossVector(Vector* v1, Vector* v2){
    return{ v1->y * v2->z - v2->y * v1->z,
        v1->z * v2->x - v2->z * v1->x,
        v1->x*v2->y - v1->y * v2->x };
};

inline Vector scalarVector(Vector* v, double scale) {
    Vector v1;
    v1.x = v->x * scale;
    v1.y = v->y * scale;
    v1.z = v->z * scale;

    return v1;
};

inline Vector normalize(Vector* v){
    Vector v1;
    double l = vectorLength(v);
    v1.x = v->x / l;
    v1.y = v->y / l;
    v1.z = v->z / l;
    return  v1;
};

typedef struct {
    Vector pos;
    Vector velocity;
    Vector acceleration;   // ?

    double damping;
    double inverseMass;
    Vector  forceAccum; // 存储下一次模拟迭代的力，每次积分后清零
} Particle;


inline void particleIntegrate(Particle* p, double duration) {
    assert(duration > 0.0);

    addVector(&p->pos, &scalarVector(&p->velocity, duration));
    Vector resultingAcc = p->acceleration;
    // F = ma =>  a = F * (1/m) 
    addVector(&resultingAcc, &scalarVector(&p->forceAccum, p->inverseMass));
    addVector(&p->velocity, &scalarVector(&resultingAcc, duration));

    scalarVector(& p->velocity,  pow(p->damping, duration) );

}


#endif
