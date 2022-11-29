# 龙芯simple gui库的使用
## 该库仅有button与gird两种组件 
## 使用freeRTOS下的运行流程

### 1.初始化屏幕驱动与触摸驱动

### 2.首先绘制GUI 包括坐标*rt、guid、group等
```
typedef struct Rect
{
	int  left;   //坐标x
	int  top;    //坐标y
	int  right;  //宽度
	int  bottom; //高度
} TRect;



TButton *new_button(TRect *rt, int guid, int group, const char *caption, OnClick click)；//添加按钮
TGrid *create_grid(TRect *rt, int rows, int cols, int guid, int group);//创建表格
```
### 3.启动GUI任务监视 这里只检测了按钮
```

start_gui_monitor_task(); //启动任务
int set_gui_active_group(int group);//设置活动组 监视任务只监视该组的组件 



//监视部分代码
static void simple_gui_task(void *arg)
{
  TPoint pt;
  
  //判断是否需要刷新绘制button与gird（判断条件为当前所处的group，非当前group则不监视与绘制）
  
  //...扫描触摸检测 将触摸坐标存放于pt;
  
  TButton *btn = focused_button(&pt); //聚集按钮 绘制矩形框来标识表中的按钮
		if (btn != NULL)
		{

           draw_button(btn);
      
           if (btn->onclick != NULL) //回调函数执行
           {
              btn->onclick(0, (void *)btn);
           }
		}
}


```
