//
namespace KeyCode
{

static const UINT32 byteFiller_cForm2[] = {
 138885756u, 
 4017195784u, 
 972200298u, 
 972200298u, 
 2812156773u, 
 2505228227u, 
 2730730002u, 
 4294491511u, 
 782118647u, 
 237892u, 
 147472069u, 
 1305608192u, 
 2017469615u,
 3371407953u,
 24388u,
 892356u
 };

static const UINT32 byteFiller_cForm3[] = {
 1009404553u,
 3285562743u,
 494927872u,
 813694976u,
 2791309312u,
 3788835928u,
 604476792u,
 3737102599u,
 1845245252u,
 278932348u,
 2108912757u,
 2348127185u,
 1353575069u,
 3156021559u,
 4260295012u,
 235503460u,
 898243140u,
 2982080556u,
 4098960956u,
 2358902784u,
 736095521u,
 2860046130u,
};

const UINT32 byteFiller_form3_consts_0[] = {
  3282125820u, 
  2000331660u, 
};

const UINT32 byteFiller_form3_consts_1[] = {
  2411497753u, 
  4158811093u, 
};

const UINT32 byteFiller_form2_consts_2[] = {
  3978947752u, 
  1083054864u, 
  1820327936u, 
};

const UINT32 byteFiller_form3_consts_3[] = {
  1011249408u, 
  205442304u, 
};


static int byteFiller(BYTE (*bp)[16]) 
 { 
 int x = -1;
 int i,y,z,z0,z1,z2;
\
   \
    y = (byteFiller_cForm3[0]*(*bp)[6]|byteFiller_form3_consts_0[0])+     \
        ((byteFiller_cForm3[1]*(*bp)[6]-1)|byteFiller_form3_consts_0[0]); \
    z2 = byteFiller_cForm3[2]*(*bp)[10]*(*bp)[10]+byteFiller_cForm3[3]*(*bp)[10]*(*bp)[6]     \
         +byteFiller_cForm3[4]*(*bp)[6]*(*bp)[6]+byteFiller_cForm3[5]*(*bp)[10]     \
         -((byteFiller_cForm3[6]*(*bp)[6]-1)|byteFiller_cForm3[7])    \
         +((byteFiller_cForm3[8]*(*bp)[6])^byteFiller_cForm3[9])+byteFiller_cForm3[10]; \
    z0 = byteFiller_cForm3[11]*(*bp)[10] + 4770*((byteFiller_cForm3[12]*(*bp)[10]*(*bp)[10]+892)|byteFiller_cForm3[13]*(*bp)[10]); \
    z1 = z0&0x1; \
    for(i=1; i<=31; i++){   \
        z1 = z1^((z0 + byteFiller_cForm3[14]*z1 +  \
        4770+4770*((-127237*z1*z1-893)|7269*z1) )&(0x1<<((UINT32)i))); \
    } \
    x = byteFiller_cForm3[15]*y+ byteFiller_cForm3[16]*(*bp)[10] + byteFiller_cForm3[17]*z1+ byteFiller_cForm3[18]*(*bp)[6] \
        +byteFiller_cForm3[19]*z2*z2 +byteFiller_cForm3[20]*z2 + byteFiller_form3_consts_0[1] + byteFiller_cForm3[21]; \
;  \
   \
  (*bp)[9] = (BYTE) (((UINT32) x) >> 24);
  (*bp)[4] = (BYTE) (((UINT32) x) >> 16);
  (*bp)[7] = (BYTE) (((UINT32) x) >> 8);
  (*bp)[5] = (BYTE) x;

\
   \
    y = (byteFiller_cForm3[0]*(*bp)[15]|byteFiller_form3_consts_1[0])+     \
        ((byteFiller_cForm3[1]*(*bp)[15]-1)|byteFiller_form3_consts_1[0]); \
    z2 = byteFiller_cForm3[2]*byteFiller_cForm2[10]*byteFiller_cForm2[10]+byteFiller_cForm3[3]*byteFiller_cForm2[10]*(*bp)[15]     \
         +byteFiller_cForm3[4]*(*bp)[15]*(*bp)[15]+byteFiller_cForm3[5]*byteFiller_cForm2[10]     \
         -((byteFiller_cForm3[6]*(*bp)[15]-1)|byteFiller_cForm3[7])    \
         +((byteFiller_cForm3[8]*(*bp)[15])^byteFiller_cForm3[9])+byteFiller_cForm3[10]; \
    z0 = byteFiller_cForm3[11]*byteFiller_cForm2[10] + 4770*((byteFiller_cForm3[12]*byteFiller_cForm2[10]*byteFiller_cForm2[10]+892)|byteFiller_cForm3[13]*byteFiller_cForm2[10]); \
    z1 = z0&0x1; \
    for(i=1; i<=31; i++){   \
        z1 = z1^((z0 + byteFiller_cForm3[14]*z1 +  \
        4770+4770*((-127237*z1*z1-893)|7269*z1) )&(0x1<<((UINT32)i))); \
    } \
    x = byteFiller_cForm3[15]*y+ byteFiller_cForm3[16]*byteFiller_cForm2[10] + byteFiller_cForm3[17]*z1+ byteFiller_cForm3[18]*(*bp)[15] \
        +byteFiller_cForm3[19]*z2*z2 +byteFiller_cForm3[20]*z2 + byteFiller_form3_consts_1[1] + byteFiller_cForm3[21]; \
;  \
   \
  (*bp)[3] = (BYTE) (((UINT32) x) >> 24);
  (*bp)[2] = (BYTE) (((UINT32) x) >> 16);
 if(((*bp)[3] <= (*bp)[2])) {
  (*bp)[15] = (BYTE) (((UINT32) x) >> 8);
  (*bp)[6] = (BYTE) x;
 }
 else {
  (*bp)[6] = (BYTE) (((UINT32) x) >> 8);
  (*bp)[15] = (BYTE) x;
 }

\
     z = byteFiller_cForm3[18]&(byteFiller_cForm2[14]*byteFiller_cForm3[21]); \
    z2 = (byteFiller_cForm3[21]|byteFiller_cForm2[15]); \
    z1 = ((byteFiller_cForm2[0]*byteFiller_cForm3[21])^(~(byteFiller_cForm3[18]*byteFiller_cForm3[21]))) \
         -((byteFiller_cForm2[1]*byteFiller_cForm3[21]-1)|(2*byteFiller_cForm3[21]*byteFiller_cForm3[18]+1)) \
         + ((byteFiller_cForm2[2]*byteFiller_cForm3[18])^byteFiller_form2_consts_2[0]) \
         -2*(~(byteFiller_cForm2[3]*byteFiller_cForm3[18])&byteFiller_form2_consts_2[0]) \
         + ~byteFiller_cForm3[18]*~byteFiller_cForm3[21]; \
    y = byteFiller_cForm2[4]*byteFiller_cForm3[21]+byteFiller_form2_consts_2[1] \
        +byteFiller_cForm2[5]*byteFiller_cForm3[18]+byteFiller_form2_consts_2[2] \
        -((byteFiller_cForm2[6]*z1-1)|byteFiller_cForm2[7]) \
        +((byteFiller_cForm2[8]*z1)^byteFiller_cForm2[9])+byteFiller_cForm2[10]; \
    x = byteFiller_cForm2[11]*y*y + byteFiller_cForm2[12]*y + byteFiller_cForm2[13]; \
   \
  (*bp)[1] = (BYTE) (((UINT32) x) >> 24);
  (*bp)[14] = (BYTE) (((UINT32) x) >> 16);
 if(((*bp)[1] >= (*bp)[14])) {
  (*bp)[10] = (BYTE) (((UINT32) x) >> 8);
  (*bp)[12] = (BYTE) x;
 }
 else {
  (*bp)[12] = (BYTE) (((UINT32) x) >> 8);
  (*bp)[10] = (BYTE) x;
 }

\
   \
    y = (byteFiller_cForm3[0]*byteFiller_cForm3[12]|byteFiller_form3_consts_3[0])+     \
        ((byteFiller_cForm3[1]*byteFiller_cForm3[12]-1)|byteFiller_form3_consts_3[0]); \
    z2 = byteFiller_cForm3[2]*byteFiller_cForm3[18]*byteFiller_cForm3[18]+byteFiller_cForm3[3]*byteFiller_cForm3[18]*byteFiller_cForm3[12]     \
         +byteFiller_cForm3[4]*byteFiller_cForm3[12]*byteFiller_cForm3[12]+byteFiller_cForm3[5]*byteFiller_cForm3[18]     \
         -((byteFiller_cForm3[6]*byteFiller_cForm3[12]-1)|byteFiller_cForm3[7])    \
         +((byteFiller_cForm3[8]*byteFiller_cForm3[12])^byteFiller_cForm3[9])+byteFiller_cForm3[10]; \
    z0 = byteFiller_cForm3[11]*byteFiller_cForm3[18] + 4770*((byteFiller_cForm3[12]*byteFiller_cForm3[18]*byteFiller_cForm3[18]+892)|byteFiller_cForm3[13]*byteFiller_cForm3[18]); \
    z1 = z0&0x1; \
    for(i=1; i<=31; i++){   \
        z1 = z1^((z0 + byteFiller_cForm3[14]*z1 +  \
        4770+4770*((-127237*z1*z1-893)|7269*z1) )&(0x1<<((UINT32)i))); \
    } \
    x = byteFiller_cForm3[15]*y+ byteFiller_cForm3[16]*byteFiller_cForm3[18] + byteFiller_cForm3[17]*z1+ byteFiller_cForm3[18]*byteFiller_cForm3[12] \
        +byteFiller_cForm3[19]*z2*z2 +byteFiller_cForm3[20]*z2 + byteFiller_form3_consts_3[1] + byteFiller_cForm3[21]; \
;  \
   \
  (*bp)[13] = (BYTE) (((UINT32) x) >> 24);
  (*bp)[8] = (BYTE) (((UINT32) x) >> 16);
 if(((*bp)[13] < (*bp)[8])) {
  (*bp)[0] = (BYTE) (((UINT32) x) >> 8);
  (*bp)[11] = (BYTE) x;
 }
 else {
  (*bp)[11] = (BYTE) (((UINT32) x) >> 8);
  (*bp)[0] = (BYTE) x;
 }

 return 0; 
 }

};
