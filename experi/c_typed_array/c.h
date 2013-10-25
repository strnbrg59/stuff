#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int i;
    double x;
} c_struct;

typedef c_struct** c_struct_array;
void c_struct_array_new(c_struct_array*);

#ifdef __cplusplus
}
#endif
