#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "os_type.h"
#include "osapi.h"
#include "mem.h"

#include "rsa_primes.h"

const int MAX_DIGITS = 50;

struct public_key_class{
    long long modulus;
    long long exponent;
};

struct private_key_class{
    long long modulus;
    long long exponent;
};


// This should totally be in the math library.
long long gcd(long long a, long long b)
{
    long long c;
    while (a != 0)
    {
        c = a; a = b%a;  b = c;
    }

    return b;
}

long long ExtEuclid(long long a, long long b)
{
    long long x = 0, y = 1, u = 1, v = 0, gcd = b, m, n, q, r;
    while (a!=0)
    {
        q = gcd/a;
        r = gcd % a;
        m = x-u*q;
        n = y-v*q;
        gcd = a;
        a = r;
        x = u;
        y = v;
        u = m;
        v = n;
    }

    return y;
}

long long rsa_modExp(long long b, long long e, long long m)
{
    if (b < 0 || e < 0 || m <= 0)
    {
        os_printf("invalid param %lld %lld %lld \n", b, e, m);
        return 0;        
    }

    b = b % m;
    if (e == 0)
    {
        return 1;
    }

    if (e == 1)
    {
        return b;
    }

    if (e % 2 == 0)
    {
        return (rsa_modExp(b * b % m, e/2, m) % m);
    }

    if (e % 2 == 1)
    {
        return (b * rsa_modExp(b, (e-1), m) % m);
    }

    return 0;
}

// Calling this function will generate a public and private key and store them in the pointers
// it is given.
void rsa_gen_keys(struct public_key_class *pub, struct private_key_class *priv)
{
    // count number of primes in the list
    long long prime_count = sizeof(rsa_primes_table)/sizeof(rsa_primes_table[0]);

    // choose random primes from the list, store them as p,q
    long long p = 0;
    long long q = 0;

    //long long e = powl(2, 8) + 1;
    long long e = pow(2, 8) + 1;
    long long d = 0;
    long long max = 0;
    long long phi_max = 0;

    // srand(time(NULL));

    do {
        // a and b are the positions of p and q in the list
        int a =  (double)rand() * (prime_count+1) / (RAND_MAX+1.0);
        int b =  (double)rand() * (prime_count+1) / (RAND_MAX+1.0);
        p = rsa_primes_table[a];
        q = rsa_primes_table[b];
        max = p*q;
        phi_max = (p-1)*(q-1);
    }
    while(!(p && q) || (p == q) || (gcd(phi_max, e) != 1));

    // Next, we need to choose a,b, so that a*max+b*e = gcd(max,e). We actually only need b
    // here, and in keeping with the usual notation of RSA we'll call it d. We'd also like
    // to make sure we get a representation of d as positive, hence the while loop.
    d = ExtEuclid(phi_max,e);
    while(d < 0){
        d = d+phi_max;
    }

    os_printf("primes are %lld and %lld\n",(long long)p, (long long )q);
    // We now store the public / private keys in the appropriate structs
    pub->modulus = max;
    pub->exponent = e;

    priv->modulus = max;
    priv->exponent = d;
}

long long *rsa_encrypt(const char *message, const unsigned long message_size,
                       const struct public_key_class *pub)
{
    long long *encrypted = (long long *)os_malloc(sizeof(long long)*message_size);
    if (encrypted == NULL)
    {
        os_printf("malloc failed.\n");
        return NULL;
    }

    long long i = 0;
    for (i=0; i < message_size; i++)
    {
        encrypted[i] = rsa_modExp(message[i], pub->exponent, pub->modulus);
    }

    return encrypted;
}


char *rsa_decrypt(const long long *message,
                  const unsigned long message_size,
                  const struct private_key_class *priv)
{
    if (message_size % sizeof(long long) != 0)
    {
        os_printf("rsa_decrypt error \n");
        return NULL;
    }
    // We allocate space to do the decryption (temp) and space for the output as a char array
    // (decrypted)
    char *decrypted = (char *)os_malloc(message_size/sizeof(long long));
    char *temp = (char *)os_malloc(message_size);
    if ((decrypted == NULL) || (temp == NULL))
    {
        os_printf("malloc failed.\n");
        return NULL;
    }
    // Now we go through each 8-byte chunk and decrypt it.
    long long i = 0;
    for (i=0; i < message_size/8; i++)
    {
        temp[i] = rsa_modExp(message[i], priv->exponent, priv->modulus);
    }
    // The result should be a number in the char range, which gives back the original byte.
    // We put that into decrypted, then return.
    for(i=0; i < message_size/8; i++)
    {
        decrypted[i] = temp[i];
    }

    os_free(temp);

    return decrypted;
}
