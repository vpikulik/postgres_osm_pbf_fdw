
typedef struct {
    int size;
    void* data;
} ResizedBuffer;

void append_data(ResizedBuffer *data, int size, void *in);
ResizedBuffer* init_resized_buffer();
void free_resized_buffer(ResizedBuffer *data);

ResizedBuffer* zdecode(void* input, int input_size);