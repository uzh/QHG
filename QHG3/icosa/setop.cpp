#include <stdio.h>
#include <set>
#include <vector>
#include <algorithm>


void show(const char *pCaption, std::set<int> s) {
    printf("%s: [", pCaption);
    std::set<int>::const_iterator it;
    for (it = s.begin(); it != s.end(); ++it) {
        printf("%d ", *it);
    }
    printf("]\n");
}

int main(int iARgC, char*apArgV[]) {
    int iResult = 0;

    std::set<int> s1;
    std::set<int> s2;

    for (int i = 0; i < 10; ++i) {
        s1.insert(2*i);
        s2.insert(3*i);
    }
    
    std::vector<int> vUnion(2);
    std::vector<int> vIntersect(20);
    std::vector<int> vDifference(20);
    
    std::vector<int>::iterator itU = std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), vUnion.begin());
    std::vector<int>::iterator itI = std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), vIntersect.begin());
    std::vector<int>::iterator itD = std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(), vDifference.begin());

    std::set<int> sUnion(vUnion.begin(), itU);
    std::set<int> sIntersect(vIntersect.begin(), itI);
    std::set<int> sDifference(vDifference.begin(), itD);

    show("s1", s1);
    show("s2", s2);
    show("union", sUnion);
    show("intersection", sIntersect);
    show("difference", sDifference);

    return iResult;
}
