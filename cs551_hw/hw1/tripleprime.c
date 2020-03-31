/* THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
 * A TUTOR OR CODE WRITER BY OTHER STUDENTS - Changmao Li
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _seg {  /* definition of new type "seg" */
    int  bits[256];
    struct _seg  *next;
}seg  ;

typedef struct {
    seg* segpt;
    int intnum;
    int bitnum;
} coordinate;

#define BITSPERSEG (8*256*sizeof(int))
seg* head;
seg* last_seg;
int last_j=0;
int last_seg_id=0;

seg* whichseg(int j){
    if (last_j==0 || j<last_j){
        int cor_int = j/2;
        int seg_id = cor_int/BITSPERSEG;
        seg*pt = head;
        if (seg_id == 0)
            return pt;
        for(int i =0;i<seg_id; i++){
            pt = pt->next;
        }
        last_seg = pt;
        last_j = j;
        last_seg_id = seg_id;
        return pt;
    }
    else if(last_j!=0&&j==last_j){
        return last_seg;
    }
    else{
        int cor_int = j/2;
        int seg_id = cor_int/BITSPERSEG;
        if (seg_id==last_seg_id){
            return last_seg;
        }
        seg*pt = last_seg;
        for(int i=last_seg_id;i<seg_id;i++){
            pt = pt->next;
        }
        last_seg = pt;
        last_j = j;
        last_seg_id = seg_id;
        return pt;
    }
}

int whichint(int j){
    int cor_int = j/2;
    int seg_id = cor_int/BITSPERSEG;
    int int_in_seg = (cor_int - seg_id*8192)/32;
    return int_in_seg;
}

int whichbit(int j){
    int cor_int = j/2;
    int seg_id = cor_int/BITSPERSEG;
    int bit_in_seg = (cor_int - seg_id*8192)%32;
    return bit_in_seg;
}

coordinate getcoord(int j){
    seg*current_seg = whichseg(j);
    int current_int = whichint(j);
    int current_bit = whichbit(j);
    coordinate result;
    result.bitnum = current_bit;
    result.intnum = current_int;
    result.segpt = current_seg;
    return result;
}

void markcoord(coordinate c){
    seg*current_seg = c.segpt;
    current_seg->bits[c.intnum] |= 1 << (c.bitnum);
}

int testcoord(coordinate c){
    seg*current_seg = c.segpt;
    if ( (current_seg->bits[c.intnum] & (1 << (c.bitnum) ))  )
        return 1;
    else
        return 0;
}

void marknonprime(int j){
    coordinate result = getcoord(j);
    markcoord(result);
}

int testprime(int j){
    coordinate result = getcoord(j);
    if (testcoord(result) == 1)
        return 0;
    else
        return 1;
}

int whichnum(coordinate c){
    seg* current_seg = c.segpt;
    seg* pt = head;
    int seg_id = 0;
    while(pt != current_seg){
        pt = pt->next;
        seg_id += 1;
    }
    int int_num = c.intnum;
    int bit_num = c.bitnum;
    return (int_num * 32 + bit_num + seg_id*BITSPERSEG)*2 + 1;
}

coordinate incrcoord(coordinate c, int inc){
    int bitnum = c.bitnum;
    if (bitnum+inc/2 <= 31){
        c.bitnum = bitnum+inc/2;
        return c;
    }
    else{
        int add_int = (bitnum+inc/2-32)/32+1;
        c.bitnum = (bitnum+inc/2-32)%32;
        c.intnum = c.intnum + add_int;
        if (c.intnum<=255){
            return c;
        }
        else{
            int add_seg = (c.intnum-256)/256 + 1;
            c.intnum = (c.intnum-256)%256;
            seg*pt = c.segpt;
            for(int i= 0;i<add_seg;i++){
                pt=pt->next;
            }
            c.segpt =pt;
            return c;
        }
    }
}

int main(int argc, char *argv[]) {

    seg *pt;
    int      i;
    int howmany;


    if (argc == 2) sscanf(argv[1],"%d",&howmany);
    else scanf("%d",&howmany);
    int nb_seg = (howmany +BITSPERSEG -1)/BITSPERSEG;


    head= (  seg * ) malloc(sizeof(seg));
    pt=head;
    for (i=0;i<nb_seg;i++) { //Just Forward Links for Now
        pt->next = (  seg *) malloc(sizeof (seg));
        memset(pt->bits, 0, sizeof(pt->bits));
        pt=pt->next;
    }

    // sieve portion
    int k = 3;
    while ((k*k) <= howmany)
    {
        if (k % 2 == 0){
            k++;
            continue;
        }
        for(int j=2; k * j <= howmany; j++)
        {
            if (j % 2 == 0)
                continue;
            else {
                int non_prime = k * j;
                marknonprime(non_prime);
            }
        }
        k++;
    }

    int nb_old_prime = 0;
    k=3;
    while (k <= howmany){
        if (k % 2 == 0){
            k++;
            continue;
        }
        if (testprime(k)) {
            nb_old_prime++;
        }
        k++;
    }
    printf("The number of odd primes less than or equal to %d is %d\n", howmany, nb_old_prime);
    int l,m;
    int s;
    // triple prime portion
    while ((s=scanf("%d %d", &l, &m)) != EOF){
        int t;
        if ((s!=2||(t=getchar())!='\n') && t!= EOF){
            printf("Input error\n");
            int o;
            while((o=getchar())!='\n' && o!=EOF);
            continue;
        }
        if (m<=l){
            printf("No solutions\n");
            continue;
        }
        if(m<=0 || l<=0){
            printf("No solutions\n");
            continue;
        }
        if(m>=howmany || l>=howmany){
            printf("No solutions\n");
            continue;
        }
        if (m%2!=0 && l%2!=0){
            coordinate d = getcoord(2+l);
            coordinate e = getcoord(2+m);
            if (!testcoord(d)&&!testcoord(e))
                printf("1 solutions, largest (2,%d,%d)\n", 2+l, 2+m);
            else
                printf("No solutions\n");
            continue;
        }
        if (m%2==0 && l%2!=0) {
            printf("No solutions\n");
            continue;
        }
        if (m%2!=0 && l%2==0){
            printf("No solutions\n");
            continue;
        }
        int nb_solutions = 0;
        int largest_num = 0;
        k=3;
        while (k <= howmany){
            if (k % 2 == 0){
                k++;
                continue;
            }
            coordinate c = getcoord(k);
            if (!testcoord(c)){
                coordinate d = incrcoord(c, l);
                if (!testcoord(d)){
                    coordinate e = incrcoord(c, m);
                    if (!testcoord(e)){
                        if (k+m<=howmany) {
                            nb_solutions += 1;
                            if (k > largest_num)
                                largest_num = k;
                        }
                    }
                }
            }
            k++;
        }
        if (nb_solutions==0)
            printf("No solutions\n");
        else
            printf("%d solutions, largest (%d,%d,%d)\n", nb_solutions, largest_num, largest_num+l, largest_num+m);
    }

    return 0;
}
