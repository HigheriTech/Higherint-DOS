#pragma once

#define PG_SIZE 0x1000 // 页大小
#define SCRNX 1024     // 屏幕横向分辨率
#define SCRNY 768      // 屏幕纵向分辨率
// 空描述符、内核代码段描述符、内核数据段描述符、用户代码段描述符、用户数据段描述符、10个TSS描述符，共15个描述符。
#define GDT_DESC_CNT 15        // GDT中描述符数量
#define PORT_KEYDAT 0x0060     // 键盘控制器数据端口
#define PORT_KEYCMDSTA 0x0064  // 键盘控制器命令端口或状态端口
#define HEAP_BLOCK_MAX_NUM 128 // 堆内存块最多数量
#define MOUSE_EVENT_MOVE 1
#define MOUSE_EVENT_LEFT_DOWN 2
#define MOUSE_EVENT_LEFT_UP 3
#define MOUSE_EVENT_DOUBLE_CLICK 4
#define MOUSE_EVENT_RIGHT_DOWN 5
#define MOUSE_EVENT_RIGHT_UP 6
#define KERNEL_PAGE_DIR_ADDR 0x20000 // 内核页目录起始地址，必须4K对齐
#define RMENU_WIDTH 128              // 右键菜单统一宽度
#define MAX_LAYERS 16                // 图层数量最大值。
#define NULL ((void *)0)             // 空指针
typedef char *va_list;
#define va_start(ap, v) ap = (va_list)&v // 把ap指向第一个固定参数v
#define va_arg(ap, t) *((t *)(ap += 4))  // ap指向下一个参数并返回其值
#define va_end(ap) ap = NULL             // 清除ap
#define MAX_TASKS 10                     // 最大任务数量
#define MAX_LAYERS_PER_TASK 3            // 每个任务最多有多少个图层
#define MAX_TIMER 2                      // 最多设定多少个定时器。
// 选择子
#define RPL0 0
#define RPL3 3
#define TI_GDT 0
#define SELECTOR_K_CODE ((1 << 3) + (TI_GDT << 2) + RPL0) // 内核代码段选择子
#define SELECTOR_K_DATA ((2 << 3) + (TI_GDT << 2) + RPL0) // 内核数据段选择子
#define SELECTOR_U_CODE ((3 << 3) + (TI_GDT << 2) + RPL3) // 用户代码段选择子
#define SELECTOR_U_DATA ((4 << 3) + (TI_GDT << 2) + RPL3) // 用户数据段选择子
/*
PIC programmable interrupt controller 可编程中断控制器。
ICW initialization command words 初始化命令字，共4个 ICW1~ICW4，必须依次设置。
OCW operation command word 操作命令字，共3个，OCW1~OCW3，其中OCW1对应IMR。
IMR interrupt mask register 中断屏蔽寄存器，8位，对应OCW1。
*/
// 以下是PIC各寄存器对应的操作端口
#define PIC0_ICW1 0x0020
#define PIC0_OCW2 0x0020
#define PIC0_IMR 0x0021
#define PIC0_ICW2 0x0021
#define PIC0_ICW3 0x0021
#define PIC0_ICW4 0x0021
#define PIC1_ICW1 0x00a0
#define PIC1_OCW2 0x00a0
#define PIC1_IMR 0x00a1
#define PIC1_ICW2 0x00a1
#define PIC1_ICW3 0x00a1
#define PIC1_ICW4 0x00a1

struct Bitmap // 位图
{
    unsigned int size;   // 位图所占字节长度
    unsigned char *bits; // 位图指针，也就是位图的起始地址。
};
struct File // FAT16目录项结构(32B)：
{
    // 文件名 如果第一个字节为0xe5，代表这个文件已经被删除；如果第一个字节为0x00，代表这一段不包含任何文件名信息。
    unsigned char name[8];
    unsigned char ext[3]; // 扩展名
    // 属性：bit0只读文件，bit1隐藏文件，bit2系统文件，bit3非文件信息(比如磁盘名称)，bit4目录，bit5文件。
    unsigned char type;
    unsigned char reserve[10]; // 保留
    unsigned short time;       // 最后一次写入时间
    unsigned short date;       // 最后一次写入日期
    unsigned short clustno;    // 起始簇号
    unsigned int size;         // 文件大小
};
struct Fifo // 先进先出
{
    int *buf;          // 缓存区起始地址
    int size;          // 缓存区大小
    int next_r;        // 下一个数据读出位置
    int next_w;        // 下一个数据写入位置
    int free;          // 缓存区空余字节数
    int flags;         // 是否溢出
    struct Task *task; // 有数据写入时需要唤醒的任务。
};
struct SegmentDescriptor // 段描述符
{
    short limit_low;
    short base_low;
    char base_mid;
    char access_right;
    char limit_high;
    char base_high;
};
struct MouseDecoder // 鼠标数据解码
{
    int buf[3]; // 鼠标原始数据，3个字节为1组。
    int phase;  // 表示等待鼠标的第几个字节。取值范围0~3，0表示等待鼠标应答数据0xfa的状态，正常只出现一次。
    int x;      // 解析出来的鼠标横向移动的距离。
    int y;      // 解析出来的鼠标纵向移动的距离。
    int btn;    // 解析出来的鼠标3个键是否按下的数据。
};
struct Pool // 内存池
{
    struct Bitmap bitmap;    // 本内存池用到的位图结构
    unsigned int addr_start; // 本内存池所管理内存的起始地址
    unsigned int size;       // 本内存池字节容量
};
struct MemBlock // 分配出去的内存块
{
    void *vaddr; // 内存块起始地址
    int pg_cnt;  // 内存块占多少个页（释放堆内存时用）
};
struct Heap // 堆
{
    struct Pool vm_pool; // 虚拟内存池
    struct MemBlock block_record[HEAP_BLOCK_MAX_NUM];
};
struct RightMenuData // 右键菜单
{
    int x, y;          // 菜单在父图层的坐标
    int menu_num;      // 菜单项个数
    char *menus[8];    // 菜单项名称（最多8个）
    int highlight_idx; // 高亮菜单项索引，-1表示当前没有高亮的菜单项。
};
struct Cursor // 光标
{
    int on;     // 当前窗口是否开启光标
    int x0, y0; // 光标在窗口内的坐标。
};
struct Layer // 图层
{
    int *buf;          // 图层数据，复制到显存即可显示。
    int xsize;         // 图层横向大小。
    int ysize;         // 图层纵向大小。
    int vx0;           // 图层在屏幕上的横轴坐标。
    int vy0;           // 图层在屏幕上的纵轴坐标。
    int col_inv;       // 透明色。-1表示没有透明色。
    int height;        // 图层高度。-1表示隐藏。
    int flags;         // 0表示该图层未使用，1表示已使用。
    struct Task *task; // 本图层所属的任务。
    int type;          // 0：普通图层、1：主窗口、2：右键菜单、3：弹窗。
    struct Cursor cursor;
    struct RightMenuData *rmenu_data; // 右键菜单数据
};
struct LayerControl // 图层控制
{
    int *vgram;         // 显存虚拟地址，统一为0xe0000000
    unsigned char *map; // 每个字节保存一个图层的编号sid，表示屏幕上每个像素显示的是哪个图层。

    int xsize; // 屏幕横向分辨率。
    int ysize; // 屏幕纵向分辨率。
    int top;   // 最上面图层的高度。-1表示暂时没有图层。

    struct Layer *layers[MAX_LAYERS]; // 按图层高度升序排列。只放显示中的图层。
    struct Layer layers0[MAX_LAYERS]; // 存放各个图层结构体。
};
struct TSS // 任务状态段(task status segment) 104字节
{
    int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3; // 经调试发现在任务切换时cr3并不会自动保存到TSS中。
    int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    int es, cs, ss, ds, fs, gs;
    int ldtr, iomap;
};
struct Task // 任务
{
    int selector;   // 用来存放TSS在GDT中的选择子
    int status;     // 0表示未使用，1表示休眠中，2表示活动中。
    int time_slice; // 时间片
    struct TSS tss;
    struct Layer *layers[MAX_LAYERS_PER_TASK]; // 0放主窗口，1放右键菜单，2放弹窗。
    struct Fifo fifo;
    void (*exit_func)(void);      // 内核任务退出时需要执行的函数（主要用来释放动态申请的内存）
    struct Heap *heap;            // 用户任务堆
    void *app_addr;               // 用户任务本身的程序文件在内核空间的地址。
    int *page_dir;                // 用户任务页目录在内核空间的地址。
    int *page_table;              // 用户任务页表在内核空间的地址。
    void *user_stack_in_kernel;   // 用户栈内核地址
    struct File *opened_file;     // 已打开的文件（每个任务不允许同时打开多个文件）。
    char should_open_filename[8]; // 任务启动后需要打开的文件。（比如双击TXT文件而启动notepad）
};
struct Timer // 定时器
{
    // struct Timer *next_timer; //用来存放下一个即将超时的定时器地址。0表示没有下一个。
    unsigned int timeout;
    // char flags;        //定时器的状态。0表示未使用，1表示待运转，2表示运转中。
    struct Fifo *fifo; // 超时后将向该fifo发送数据
    int data;          // 准备向fifo发送的数据
};
struct TimerControl // 定时器控制
{
    unsigned int count; // 时钟中断累计次数。
    // unsigned int next_time; //所有定时器中下一个超时的时刻。
    // struct Timer *t0;       // timeout最小的定时器。
    struct Timer timers0[MAX_TIMER];
};

// main.c
extern struct Fifo sys_fifo;

// bitmap.h
void init_bitmap(struct Bitmap *btmp);
int bitmap_scan(struct Bitmap *btmp, unsigned int cnt);
void bitmap_set(struct Bitmap *btmp, unsigned int bit_idx, char value);

// disk.h
void read_disk(int lba, int sector_count, unsigned int memory_addrress);
void write_disk(int lba, int sector_count, unsigned int memory_addrress);

// explorer.h
void open_explorer(void);

// fat16.h
int read_root_dir(struct File *file_info);
void read_file(struct File *file, void *file_addr);
struct File *create_file(char *fullname);
void delete_file(struct File *file);
void alter_file_name(struct File *file, char *new_fullname);
void save_file(struct File *file, char *content);
struct File *open_file(char *fullname);
void close_file(void);

// fifo.h
void init_fifo(struct Fifo *fifo, int size, int *buf, struct Task *task);
int fifo_put(struct Fifo *fifo, int data);
int fifo_get(struct Fifo *fifo);
int fifo_length(struct Fifo *fifo);

// font.h
//  256个ASCII字符的点阵位图，在font.asm中，每个字符的位图16字节，共4096字节，每个ASCII值乘以16得到对应的点阵位图起始地址。
extern char font_ascii[4096];

// func.h 来自汇编的函数声明
// CPU停止运行，直到有中断发生。
void asm_hlt(void);
// 关中断
void asm_cli(void);
// 开中断
void asm_sti(void);
// 开中断并停止运行
void asm_stihlt(void);
// 从指定端口读取1字节数据
int io_in8(int port);
// 从指定端口读取2字节数据
int io_in16(int port);
// 向指定的端口发送1字节的数据，data的最低1字节。
void io_out8(int port, int data);
// 向指定的端口发送2字节的数据，data的最低2字节。
void io_out16(int port, int data);
// 获取eflags的值
int get_eflags(void);
// 设置eflags的值
void set_eflags(int eflags);
// 加载GDT寄存器
void load_gdtr(int limit, int addr);
// 加载IDT寄存器
void load_idtr(int limit, int addr);
// 加载任务寄存器
void load_tr(int tr);
// 远跳转，可以用来切换TSS。
void farjmp(int eip, int cs);
// 开启分页 pg_dir为页目录地址
void open_page(unsigned int *pg_dir);
// 更新tlb，删除虚拟地址对应的单条TLB缓存。
void do_invlpg(void *vaddr);
// 时钟中断处理函数
void asm_inthandler20(void);
// 键盘中断处理函数
void asm_inthandler21(void);
// 鼠标中断处理函数
void asm_inthandler2c(void);
// 系统调用
void asm_inthandler30(void);

// gdt.h
extern struct SegmentDescriptor gdt[GDT_DESC_CNT];
void init_gdt(void); // 初始化GDT
void set_segmdesc(struct SegmentDescriptor *sd, unsigned int limit, int base, int ar);

// graph.h
void fill_rectangle(struct Layer *layer, int x0, int y0, int width, int height, int color);
void print_string(struct Layer *layer, int x, int y, int color, char *str, int scale);
int gprintf(struct Layer *layer, int x, int y, int color, const char *format, ...);
void print_text(struct Layer *layer, char *str, int x0, int y0, int width, int height, int lens[]);
void init_desktop(struct Layer *layer);
void init_mouse_cursor(int *mouse, int bgcolor);

// int.h
void init_pic(void);
void init_idt(void);
void disable_interrupt();
void recover_interrupt();

// keyboard.h
void wait_KBC_sendready(void);
void init_keyboard(int data0);
void inthandler21(void);
char keyboark_decode(int key_code);

// message.h
int get_sys_msg(int is_wait);

// memory.h
extern struct Pool pm_pool;
extern struct Heap k_heap;
void init_pool(struct Pool *pool, unsigned int addr_start, unsigned int size, void *bits);
void init_memory(void);
void *palloc(struct Pool *pool, int pg_cnt);
void *kmalloc(unsigned int size);
void kfree(void *vaddr);
void *umalloc(unsigned int size);
void ufree(void *vaddr);
void free(void *vaddr, struct Heap *heap);
void pfree(struct Pool *pool, void *pg_addr, int pg_cnt);

// mouse
extern int last_mouse_event;
void init_mouse(int data0, struct MouseDecoder *mdec);
int mouse_decode(struct MouseDecoder *mdec, int dat);
void inthandler2c(void);

// page.h
void init_page(unsigned int pgram, unsigned int vgram);
void page_table_add(void *_vaddr, void *_page_phyaddr);
unsigned int addr_v2p(unsigned int vaddr);
void page_table_pte_remove(unsigned int vaddr);
unsigned int create_user_page(int file_size, void *file_addr, unsigned int *page_dir, unsigned int *page_table, unsigned int *user_stack_in_kernel);
void add_wait_free_user_page_dir(void *page_dir);
void free_wait_free_user_page_dirs(void);

// right_menu.h
void *open_right_menus(struct RightMenuData *menu_data);
void close_right_menus(void);
void highlight_right_menu(int idx);

// layer.h
extern struct LayerControl *layerctl;
void init_layer(int *vgram, int xsize, int ysize);
struct Layer *layer_alloc(void);
void layer_set(struct Layer *layer, int *buf, int xsize, int ysize, int col_inv);
void layer_updown(struct Layer *layer, int height);
void layer_refresh(struct Layer *layer, int bx0, int by0, int width, int height);
void layer_slide(struct Layer *layer, int vx0, int vy0);
void layer_free(struct Layer *layer);
void change_color(struct Layer *layer, int x0, int y0, int width, int height, int old_color, int new_color);

// stdio.h
unsigned int vsprintf(char *str, const char *format, va_list ap);

// string.h
void memset(void *dst_, unsigned char value, unsigned int size);
void memcpy(void *dst_, const void *src_, unsigned int size);
int memcmp(const void *a_, const void *b_, unsigned int size);
char *strcpy(char *dst_, const char *src_);
unsigned int strlen(const char *str);

// syscall.h
int syscall(int *esp_user, int api_num);

// task.h
extern struct Task *main_task;
extern struct Timer *task_timer; // 任务切换定时器
void init_task(void);
void task_add(struct Task *task);
void task_sleep(struct Task *task);
void task_switch(void);
struct Task *task_now(void);
struct Task *new_kernel_task(void *entry_func, void *exit_func);
struct Task *new_user_task(struct File *file, char *should_open_filename);
void exit_task(struct Task *task);
void exit_current_task(void);
void get_should_open_filename(char *filename);

// timer.h
extern struct TimerControl timerctl;
void init_pit(void);
// struct Timer *timer_alloc(void);
void timer_set_fifo(struct Timer *timer, struct Fifo *fifo, int data);
void timer_set_time(struct Timer *timer, unsigned int timeout);
void inthandler20(void);

// window.h
extern struct Timer *cursor_timer;
extern struct Layer *focus_win;
void *open_popwin(char *title, char *msg);
void *open_window(int width, int height, char *title);
void cursor_blink(struct Layer *window, int isvisible);
void switch_cursor(struct Layer *window, int on);
void move_cursor(struct Layer *window, int x, int y);
// void change_title_bar_color(struct Layer *layer, int is_focus);
void focus_exchange(struct Layer *old_focus_layer, struct Layer *new_focus_layer);
void init_cursor(void);

#define EXPLORER_WIDTH 800
#define EXPLORER_HEIGHT 600
#define INPUT_NAME_BOX_WIDTH 8 * 12 + 1 //加1是为了放光标
#define INPUT_NAME_BOX_HEIGHT 16
#define SELECTED_ITEM_COLOR 0x00CCE8FF //204, 232, 255 鼠标选中文件时的背景色
#define NORMAL_BG_COLOR COL8_FFFFFF

static struct File *file_infos;
static int file_num;
static char fullname[13];
static int selected_file_index;

//以下4个变量是选中文件列表项的背景范围
static const int item_x0 = 1;
static int item_y0;
static const int item_width = EXPLORER_WIDTH - 2;
static const int item_height = 16;

//鼠标相关变量
static int mouse_data_stage = 0; //0~2，鼠标数据是3个连续的int。
static int mouse_event = 0;
static int mouse_x = 0;
static int mouse_y = 0;

static int rmenus_idx = -1;   //当前显示哪个右键菜单，-1表示当前没有显示右键菜单。
static int is_input_mode = 0; //是否是在输入文件名模式
static int fullname_idx = 12; //文件名字符索引 0~12
//输入文件名文本框坐标
static const int input_box_x = 1;
static int input_box_y;
static int cursor_x, cursor_y; //光标坐标

static void init(struct Layer *win)
{
    memset(fullname, 0, 13);
    selected_file_index = -1;
    item_y0 = selected_file_index * 16 + 36;

    mouse_data_stage = 0;
    mouse_event = 0;
    mouse_x = 0;
    mouse_y = 0;

    rmenus_idx = -1;
    is_input_mode = 0;
    fullname_idx = 12;
    input_box_y = selected_file_index * 16 + 36;
    cursor_x = fullname_idx * 8 + 1;
    cursor_y = input_box_y;

    close_right_menus();
    switch_cursor(win, 0);
}

/**从硬盘读取文件信息，刷新窗口。返回文件数量。
 * explorer_win 窗口指针
 * file_infos 将从硬盘读取的文件列表信息存储到该数组中。
 */
void refresh(struct Layer *win)
{
    init(win);
    char name[9] = {0};
    char ext[4] = {0};
    file_num = read_root_dir(file_infos);
    fill_rectangle(win, 1, 20, win->xsize - 2, win->ysize - 21, COL8_FFFFFF);
    gprintf(win, 1, 20, COL8_000000, "name            size");
    for (int i = 0; i < file_num; i++)
    {
        memcpy(name, file_infos[i].name, 8);
        memcpy(ext, file_infos[i].ext, 3);
        gprintf(win, 1, i * 16 + 36, COL8_000000, "%s.%s    %d", name, ext, file_infos[i].size);
    }
}

void explorer_exit(void)
{
    kfree(file_infos);
}

void explorer_work(void)
{
    struct Layer *explorer_win = open_window(EXPLORER_WIDTH, EXPLORER_HEIGHT, "explorer");
    file_infos = kmalloc(sizeof(struct File) * 16);
    refresh(explorer_win);

    //右键菜单
    struct RightMenuData rmenus_data[2];
    rmenus_data[0].menu_num = 2; //菜单1
    rmenus_data[0].menus[0] = "new file";
    rmenus_data[0].menus[1] = "refresh";
    rmenus_data[1].menu_num = 2; //菜单2
    rmenus_data[1].menus[0] = "rename";
    rmenus_data[1].menus[1] = "delete";

    for (;;)
    {
        int msg = get_sys_msg(1);
        if (256 <= msg && msg <= 511) //键盘
        {
            int key = msg - 256;
            if (is_input_mode && key > 0)
            {
                if (key == '\n') //回车保存输入的文件名
                {
                    is_input_mode = 0;
                    switch_cursor(explorer_win, 0);
                    struct File *file = &file_infos[selected_file_index];
                    alter_file_name(file, fullname);
                    refresh(explorer_win);
                    continue;
                }
                else if (key == '\b' && 1 <= fullname_idx && fullname_idx <= 12)
                {
                    fullname_idx--;
                    fullname[fullname_idx] = 0;
                }
                else if (key != '\b' && 0 <= fullname_idx && fullname_idx <= 11)
                {
                    if ('a' <= key && key <= 'z')
                    {
                        key -= 0x20;
                    }
                    fullname[fullname_idx] = key;
                    fullname_idx++;
                }
                cursor_x = fullname_idx * 8 + 1;
                move_cursor(explorer_win, cursor_x, cursor_y);
                fill_rectangle(explorer_win, input_box_x, input_box_y, INPUT_NAME_BOX_WIDTH, INPUT_NAME_BOX_HEIGHT, COL8_FFFFFF);
                print_string(explorer_win, input_box_x, input_box_y, COL8_000000, fullname, 1);
            }
        }
        else if (512 <= msg && msg <= 512 + 1023) //鼠标
        {
            msg -= 512;
            if (mouse_data_stage == 0 && msg < 7)
            {
                mouse_event = msg;
                mouse_data_stage++;
            }
            else if (mouse_data_stage == 1)
            {
                mouse_x = msg;
                mouse_data_stage++;
            }
            else if (mouse_data_stage == 2)
            {
                mouse_y = msg;
                mouse_data_stage = 0;

                if (rmenus_idx >= 0) //当有右键菜单时
                {
                    //以下4个变量是右键菜单在本窗口中的坐标范围
                    int rmx0 = rmenus_data[rmenus_idx].x;
                    int rmy0 = rmenus_data[rmenus_idx].y;
                    int rmx1 = rmx0 + RMENU_WIDTH;
                    int rmy1 = rmenus_data[rmenus_idx].menu_num * 16 + rmy0;
                    if (rmx0 <= mouse_x && mouse_x < rmx1 && rmy0 <= mouse_y && mouse_y < rmy1) //当鼠标在右键菜单上时
                    {
                        int ridx = (mouse_y - rmy0) / 16;    //鼠标在哪个菜单项上方。
                        if (mouse_event == MOUSE_EVENT_MOVE) //当鼠标在右键菜单上移动时，相应的菜单项要高亮。
                        {
                            highlight_right_menu(ridx);
                            continue;
                        }
                        else if (mouse_event == MOUSE_EVENT_LEFT_DOWN) //鼠标点击某个菜单项
                        {
                            if (rmenus_idx == 0 && ridx == 0) //新建文件
                            {
                                void *file = create_file("NEW FILE.TXT");
                                if (file == 0)
                                {
                                    open_popwin("message", "File name already exist.");
                                    continue;
                                }
                                else
                                {
                                    refresh(explorer_win);
                                    continue;
                                }
                            }
                            else if (rmenus_idx == 0 && ridx == 1) //刷新
                            {
                                refresh(explorer_win);
                                continue;
                            }
                            else if (rmenus_idx == 1 && ridx == 0) //重命名
                            {
                                is_input_mode = 1;
                                fullname_idx = 12;
                                input_box_y = selected_file_index * 16 + 36;
                                cursor_x = fullname_idx * 8 + 1;
                                cursor_y = input_box_y;
                                memcpy(fullname, file_infos[selected_file_index].name, 8);
                                fullname[8] = '.';
                                memcpy(fullname + 9, file_infos[selected_file_index].ext, 3);
                                switch_cursor(explorer_win, 1);
                                move_cursor(explorer_win, cursor_x, cursor_y);
                                fill_rectangle(explorer_win, input_box_x, input_box_y, INPUT_NAME_BOX_WIDTH, INPUT_NAME_BOX_HEIGHT, COL8_FFFFFF);
                                print_string(explorer_win, input_box_x, input_box_y, COL8_000000, fullname, 1);
                                continue;
                            }
                            else if (rmenus_idx == 1 && ridx == 1) //删除
                            {
                                delete_file(&file_infos[selected_file_index]);
                                refresh(explorer_win);
                                continue;
                            }
                        }
                    }
                }
                int item_index = (mouse_y - 20) / 16 - 1;     //鼠标在哪个文件列表项上
                if (item_index >= 0 && item_index < file_num) //当鼠标在某个文件上时。
                {
                    if (mouse_event == MOUSE_EVENT_LEFT_DOWN) //当鼠标单击某个文件时。
                    {
                        if (selected_file_index != item_index)
                        {
                            if (selected_file_index >= 0)
                            { //将之前选中的文件恢复背景色。
                                item_y0 = selected_file_index * 16 + 36;
                                change_color(explorer_win, item_x0, item_y0, item_width, item_height, SELECTED_ITEM_COLOR, NORMAL_BG_COLOR);
                            }
                            selected_file_index = item_index;
                            item_y0 = selected_file_index * 16 + 36;
                            change_color(explorer_win, item_x0, item_y0, item_width, item_height, NORMAL_BG_COLOR, SELECTED_ITEM_COLOR);
                        }
                    }
                    else if (mouse_event == MOUSE_EVENT_DOUBLE_CLICK) //当鼠标双击某个文件时。
                    {
                        struct File *selected_file = &file_infos[selected_file_index];
                        if (memcmp("EXE", selected_file->ext, 3) == 0)
                        { //如果双击后缀名为EXE的文件，则运行该文件。
                            struct Task *task = new_user_task(selected_file, 0);
                            if (task == 0)
                            {
                                open_popwin("message", "new user task fail.");
                            }
                        }
                        else if (memcmp("TXT", selected_file->ext, 3) == 0)
                        { //如果双击后缀名为TXT的文件，则用记事本打开该文件。
                            struct File *notepad = 0;
                            for (int i = 0; i < 16; i++)
                            {
                                notepad = &file_infos[i];
                                if (memcmp(notepad->name, "NOTEPAD ", 8) == 0 && memcmp(notepad->ext, "EXE", 3) == 0)
                                {
                                    struct Task *task = new_user_task(notepad, (char *)selected_file->name);
                                    if (task == 0)
                                    {
                                        open_popwin("message", "new user task fail.");
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    else if (mouse_event == MOUSE_EVENT_RIGHT_DOWN) //当鼠标右键某个文件时弹出对应的右键菜单
                    {
                        rmenus_idx = 1;
                        rmenus_data[rmenus_idx].x = mouse_x;
                        rmenus_data[rmenus_idx].y = mouse_y;
                        open_right_menus(&rmenus_data[rmenus_idx]);
                    }
                }
                else if (item_index >= file_num) //当鼠标在文件列表以下的空白区域时。
                {
                    if (mouse_event == MOUSE_EVENT_LEFT_DOWN)
                    {
                        rmenus_idx = -1;
                        close_right_menus();
                    }
                    else if (mouse_event == MOUSE_EVENT_RIGHT_DOWN) //当鼠标右键空白区域时弹出对应的右键菜单
                    {
                        rmenus_idx = 0;
                        rmenus_data[rmenus_idx].x = mouse_x;
                        rmenus_data[rmenus_idx].y = mouse_y;
                        open_right_menus(&rmenus_data[rmenus_idx]);
                    }
                }
                else if (item_index < 0) //当鼠标在第1个文件以上的区域时
                {
                    if (mouse_event == MOUSE_EVENT_LEFT_DOWN)
                    {
                        rmenus_idx = -1;
                        close_right_menus();
                    }
                    else if (mouse_event == MOUSE_EVENT_RIGHT_DOWN)
                    {
                        rmenus_idx = -1;
                        close_right_menus();
                    }
                }
            }
        }
    }
}
