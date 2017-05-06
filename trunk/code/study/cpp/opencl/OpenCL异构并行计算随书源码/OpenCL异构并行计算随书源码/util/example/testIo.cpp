#include "../inc/yyfnutil.h"
class Pair{
    int id;
    float value;
public:
    Pair(){}
    Pair(int id, float value): id(id), value(value){}

    friend istream& operator >> (istream& in, Pair& p){
        in >> p.id >> p.value;
        return in;
    }
    friend ostream& operator <<(ostream& out, const Pair& p){
        out <<p.id<< " "<<p.value;
        return out;
    }
};

int main(){
    Pair *p;
//    float *p ;
    p = NULL;
    size_t len;
    
    IO::readFile("f.out", &p, &len);
    
//    IO::writeFile("y.out", p, len);
    IO::println(len, 5, p);
    printf("len = %d\n", len);
    free(p);

    return 0;
}
