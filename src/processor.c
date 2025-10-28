#include "processor.h"
#include "global_image_buffer.h"
#include <string.h>
#include <stddef.h>
#include "image.h"
#include "dynamic_log.h"
#ifdef PROCESSOR_VERIFY_BINARY
#include <assert.h>
#endif

#ifndef RESTRICT
#if defined(__GNUC__) || defined(__clang__)
#define RESTRICT __restrict__
#elif defined(_MSC_VER)
#define RESTRICT __restrict
#else
#define RESTRICT
#endif
#endif

#define loop_forward_far 100
#define loop_forward_near 20
#define forward_near 1 //我的数据
//声明  
void left_ring_confirm();

//求绝对值
int abs(int value)
{
    if(value>=0) return value;
    else return -value;
}

/*函数名称：void find_angle_left_down(int*angle_x,int*angle_y)
功能说明：抓住左上第二角点
*/
void find_angle_left_down(int*angle_x,int*angle_y)
{
    /*
    //生长方向
    int x=*angle_x, y=*angle_y;
    while(Grayscale[119-y][x]!=0&&y<110)
    {
        y++;
    }
    while(Grayscale[119-y][x+1]!=255&&x<187)
    {
        x++;
    }
    while(y>40)
    {

        if(dir_l[y]==4&&dir_l[y-1]==5)//向上
        {
            break;
        }
        else if(dir_l[y]==4)
        {
            y--;
        }
        else if(dir_l[y]==5)//右上
        {
            y++;
        }
        
    }
    *angle_x=x;
    *angle_y=y;
    */
    // 安全实现：严格先判范围再访问像素，去除魔法数，使用 image_w/image_h
    int x = *angle_x;
    int y = *angle_y;
    const int W = image_w;
    const int H = image_h;

    // Clamp 初值，防御性处理
    if (x < 0) x = 0; else if (x >= W) x = W - 1;
    if (y < 0) y = 0; else if (y >= H) y = H - 1;

    // 扫描到当前列上的黑点（向下增大 y），上限留出 10 行裕量，等价原来的 y<110（当 H=120）
    const int y_top_limit = (H >= 10) ? (H - 10) : H - 1;
    while (y < y_top_limit) {
        int row = H - 1 - y;                 // 行索引转换：自下而上到自上而下
        if (row < 0 || row >= H) break;      // 保护
        if (Grayscale[row][x] == 0) break;   // 命中黑点
        y++;
    }

    // 向右找到“黑→白”边界（确保不越界）
    while (x + 1 < W) {
        int row = H - 1 - y;
        if (row < 0 || row >= H) break;
        if (Grayscale[row][x + 1] == 255) break; // 右邻为白则停
        x++;
    }

    // 自上而下持续抓取该特征点：优先保持 x，其次±1/±2/±3 的黑点邻域，逐行上移
    // 保持与原逻辑一致：每轮 y--
    const int y_bottom_limit = 40; // 继承原阈值，如需参数化可提升为常量/配置
    while (y > y_bottom_limit) {
        int row = H - 1 - y;
        if (row < 0 || row >= H) break;

        if (Grayscale[row][x] == 0) {
            // 命中，保持 x 不变
        } else if (x - 1 >= 0 && Grayscale[row][x - 1] == 0) {
            x -= 1;
        } else if (x + 1 < W && Grayscale[row][x + 1] == 0) {
            x += 1;
        } else if (x - 2 >= 0 && Grayscale[row][x - 2] == 0) {
            x -= 2;
        } else if (x + 2 < W && Grayscale[row][x + 2] == 0) {
            x += 2;
        } else if (x - 3 >= 0 && Grayscale[row][x - 3] == 0) {
            x -= 3;
        } else if (x + 3 < W && Grayscale[row][x + 3] == 0) {
            x += 3;
        } else {
            break; // 本行附近未找到黑点，结束
        }
        y--; // 继续向上抓
    }

    // 回写并最终夹取
    if (x < 0) x = 0; else if (x >= W) x = W - 1;
    if (y < 0) y = 0; else if (y >= H) y = H - 1;
    *angle_x = x;
    *angle_y = y;
    
}
/*函数名称：void left_ring_first_angle()
功能说明：左环第一角点
*/
void left_ring_first_angle()
{
for(int y=loop_forward_near;y<loop_forward_far;y++)//逐行扫描
    {
        if(
            //lineinfo[y + 3].left_lost
            //&&lineinfo[y + 2].left_lost&&
            ((left_lost[y + 1])||((l_border[y]-l_border[y+1])>=5*(l_border[y+1]-l_border[y+2])))///////////
            //&& !left_lost[y - 3]
            && !left_lost[y - 2]
            && !left_lost[y - 1]
            && !left_lost[y]
            && !right_lost[y + 5]
            && !right_lost[y + 4]
            && !right_lost[y + 3]
            && !right_lost[y + 2]
            && !right_lost[y + 1]
            && !right_lost[y]
            && !right_lost[y - 1]
            && !right_lost[y - 2]
            && !right_lost[y - 3]
            && !right_lost[y - 4]
            && !right_lost[y - 5]

            && l_border[y] - l_border[y + 4] > 10
            && y < watch.InLoopAngleL
            //&&lineinfo[y].left>=lineinfo[y-2].left
            && y < 75
            )
        {//左圆环的第一个角点所在行
            watch.InLoopAngleL = y;
            left_ring_confirm();//10.25testtag
            if(0)     //在当前无元素时进行以下操作，其他时候只找角点
            {
                   //left_ring_confirm();
            }
            break;
        }
    }
}

/*函数名称：void right_ring_first_angle()
左环二次确认函数
*/
void left_ring_confirm()
{
    uint8_t zebra_confirm=0,white_count1=0,white_count2=0,white_count3=0,black_count=0,right_lost=0;
    //right_ring_first_angle();//扫描是否存在右环角点
    //left_ring_circular_arc();//扫描是否存在左环上弧
    for(int y=loop_forward_near;y<95;y++)//逐行扫描
    {
        if(((r_border[y+2]-r_border[y])>2)||(r_border[y]-r_border[y+2])>4)
            right_lost+=1;
//        if(lineinfo[y].right-lineinfo[y+2].right>20
//           &&lineinfo[y-1].right-lineinfo[y+3].right>20
//           &&lineinfo[y-2].right-lineinfo[y+4].right>20)
//            right_lost+=3;
    }
       if(right_lost<3)
       {
//           for(int x=lineinfo[watch.InLoopAngleL-1].left;x>0;x--)
//           {
//               if(Grayscale[119-watch.InLoopAngleL][x]==255)
//                   white_count1++;
//               if(Grayscale[119-(watch.InLoopAngleL-1)][x]==255)
//                   white_count2++;
//               if(Grayscale[119-(watch.InLoopAngleL-2)][x]==255)
//                   white_count3++;
//           }
           //vofa.loop[5]=white_count1;
           //vofa.loop[6]=white_count2;
           //vofa.loop[7]=white_count3;
           for(int y=watch.InLoopAngleL;y>loop_forward_near;y--)
           {
               if(Grayscale[119-y][l_border[watch.InLoopAngleL]]==0)
                  black_count++;
           }
		   //老学长上位机
			//           vofa.loop[6]=l_border[watch.InLoopAngleL].left;
		   //           vofa.loop[7]=black_count;   
           if(/*(white_count1>=10&&white_count2>=10&&white_count3>=10)&&*/(black_count<10))
           {
			   //状态机
               //enter_element(Left_ring);    //正式进入左圆环元素
			   //编码器积分
               //begin_distant_integeral(6000);
              // if(Element_rem.loop_data[Element_rem.loop_count]==0)//如果是小环
			   //牢学长的石
//               {
//                   set_speed(setpara.loop_target_speed);
//                   mycar.pid_ctrl=0;
//                   change_pid_para(&CAM_Turn,&setpara.loop_turn_PID);//将转向PID参数调为环内转向PID
//               }
/*               else//如果是大环
               {
                   set_speed(setpara.big_loop_speed);
                   //change_pid_para(&CAM_Turn,&setpara.big_loop_PID);
               }*/
               watch.InLoop = 1;
               //beep2(1,20);//蜂鸣器
               return;
           }
       }
	   //状态机
    //Element=None;
    watch.InLoopAngleR=120;
    watch.InLoopAngleL=120;
}
/*函数名称：void left_ring_circular_arc()
/*功能说明：左环上凸弧扫描函数
*/
void left_ring_circular_arc()
{
    if (watch.InLoop != 1&&watch.InLoop != 2)return;//在循环之前跳出，节省时间
    //beep(20);
    for(int y=loop_forward_near;y<loop_forward_far;y++)//逐行扫描
    {
        if (y <watch.InLoopAngle2  
            &&(watch.InLoopAngleL<65)//去除了两个积分条件
           //&&(y>(watch.InLoopAngleL+20))
           &&y <watch.InLoopCirc   //初始化给120
           &&!left_lost[y+3]
           &&!left_lost[y+2]
           &&!left_lost[y+1]
           &&!left_lost[y-3]
           &&!left_lost[y-2]
           &&!left_lost[y-1]
           &&l_border[y+1] <= l_border[y]
           &&l_border[y+2] <= l_border[y]
           &&l_border[y+3] <= l_border[y]
           &&l_border[y-1] <= l_border[y]
           &&l_border[y-2] <= l_border[y]
           &&l_border[y-3] <= l_border[y]
           //&&(watch.right_lost+watch.cross_lost)<5
            )
       { //入环点所在行
            watch.InLoopCirc = y;
            //beep(20);
            break;
       }
    }
}
/*函数名称：void left_ring_second_angle()
功能说明：左环第二角点检测函数
*/
void left_ring_second_angle()
{
    if(watch.InLoop != 1&&watch.InLoop != 2)return;//在循环之前跳出，节省时间
    for(int y=loop_forward_far;y>loop_forward_near;y--)//逐行扫描
    {
        if (//watch.InLoopCirc<66&&
            y<watch.InLoopAngle2
             &&watch.InLoopAngle2==120
             //&&get_integeral_state(&distance_integral)==2
           &&y > 60
           &&y < (loop_forward_far-2)
           &&y>watch.InLoopCirc
           &&l_border[y+1] > 30
           &&(l_border[y+1]-l_border[y])<=2
           &&(l_border[y]-l_border[y-4])>l_border[y]/2
           )
           {
               watch.InLoopAngle2 = y;
               watch.InLoopAngle2_x=l_border[watch.InLoopAngle2];
               //if()
               //watch.InLoopCirc=0;
               break;
           }
    }
	//持续抓住第二角点，保证补线完整
    if(watch.InLoopAngle2!=120
        &&watch.InLoopAngle2>50
        )
    {
        find_angle_left_down(&watch.InLoopAngle2_x,&watch.InLoopAngle2);
    }
}
/*函数名称：void left_ring_begin_turn()
功能说明：左环开始转向状态机函数
*/
void left_ring_begin_turn()
{
	//去除了路径积分和角度积分
    if(watch.InLoop!=1)return;//在循环之前跳出，节省时间
    if(/*get_integeral_state(&distance_integral)==2 路程积分完成
        &&*/watch.InLoop==1
        &&watch.InLoopAngle2<=90  //注意，该值影响补线入环的早晚
    )
    {
        //clear_distant_integeral();//清除路程积分变量
        watch.InLoop=2;
        //set_speed(setpara.loop_target_speed);
        //change_pid_para(&CAM_Turn,&setpara.loop_turn_PID);//将转向PID参数调为环内转向PID
        //watch.fix_slope=(float)(lineinfo[watch.InLoopAngle2].left)/(115-watch.InLoopAngle2);
        //begin_angle_integeral(260);
        //beep2(2,20);
    }
}
/*函数名称：left_ring_prepare_out()
功能说明：小车角度积分完成，准备出环
*/
void left_ring_prepare_out()//第340帧
{
    if(watch.InLoop != 3)return;
    if( watch.InLoop == 3
        //&&get_integeral_state(&angle_integral)==1 10.28 test tag
        //&&get_integeral_data(&angle_integral)>160
        &&r_border[69]<120
        &&r_border[69]>95
    )
   {
       watch.InLoop = 4;
       watch.OutLoop_turn_point_x=r_border[69];
       //beep2(4,20);
   }
}
/*函数名称：left_ring_out_angle()
功能说明：检测出环时右角点位置
*/
void left_ring_out_angle()
{
    if(watch.InLoop != 4)return;//在循环之前跳出，节省时间
    for(int y = loop_forward_far;y>loop_forward_near;y--)//逐行扫描
        {
        if ((watch.InLoop == 4)&&y<80
                 //lineinfo[y].left_lost
                 &&r_border[y+1] >= r_border[y]
                 &&r_border[y+2] >= r_border[y+1]
                 &&r_border[y-1] >= r_border[y]
                 &&r_border[y-2] >= r_border[y]
/*                 &&lineinfo[y - 3].right > lineinfo[y - 1].right
                 &&lineinfo[y + 4].right > lineinfo[y + 2].right
                 &&lineinfo[y - 5].right > lineinfo[y - 3].right*/
                 &&r_border[y] > 30
                 &&Grayscale[119-y-2][r_border[y]]==255
)
             {
                 if(watch.OutLoopAngle1>y)
                 {
                     //watch.OutLoopRight = lineinfo[y].right;
                     watch.OutLoopAngle1 = y; //出环判断列
                     break;
                 }
             }
        }
}
/*----------------------------------补线---------------------------------------------------------------------*/
float slopeL;
float slopeOL;
float slopeR;
float slopeOR;
float slopeJ;
float slopeCL;
float slopeCR;
/*
void left_ring_linefix()
补线函数
*/
void left_ring_linefix()
{
    uint16_t xl,xr;
    //vofa.loop[2]=watch.watch_lost;
    for (uint8_t y = forward_near; y <= 119; y++)//120原为赛道最远端
    {
        xl = l_border[y];
        xr = r_border[y];
        if (watch.InLoop == 1 && watch.InLoopAngleL < watch.InLoopCirc
              && watch.zebra_flag == 0
              && y < 81 && watch.InLoopAngle2 == 120)
           {// 先拉一道实现封住出口,由于左边丢线右边不丢线,故以右边为参考补左边线
              slopeL=(float)(r_border[2]-r_border[80])/80;//x=k*y
              watch.top_x=r_border[0]-118*slopeL;
              slopeL=(float)(watch.top_x-l_border[0])/118;
               xl = watch.top_x-slopeL*(118-y);
           }

           // 开始入左环
           // 入环点为了解决从右边沿拉线导致，打角不稳问题
          else if(watch.InLoop == 2)
          {
              //slopeR=(float)(lineinfo[40].right-watch.InLoopAngle2_x)/(watch.InLoopAngle2-40);
              slopeR=(float)watch.InLoopAngle2_x/(115-watch.InLoopAngle2);//115是左顶点纵坐标
              xr=(uint16_t)(slopeR*(watch.InLoopAngle2-y)+watch.InLoopAngle2_x);
              if(xr>186)xr=186;
              if(y>watch.InLoopAngle2||watch.InLoopAngle2<70)xl=0;
          }
          else if(watch.InLoop == 3)
          {
              if(y>50)xl=0;
          }
           // 开始出左环
           else if (watch.InLoop == 4 )
           {
               if(y>50)xl=0;
               if(r_border[watch.OutLoopAngle1] > 60 && y > watch.OutLoopAngle1)
               {
               // 一元一次方程,参考图片/出左环.png
               xr=watch.OutLoop_turn_point_x+(69-y);
               }
               //begin_angal_integeral(50);
           }
           // 出左环直行
           else if (watch.InLoop == 5
                   &&watch.OutLoopAngle2==120
                   &&watch.zebra_flag == 0)
           {// 封住入环口,补线思路是从角点向下拉线到near右边沿减145的地方
               // xl = lineinfo[y].right - 132 + y;
               slopeL=(float)(r_border[45]-r_border[75])/30;
               watch.top_x=r_border[45]-73*slopeL;
               slopeL=(float)(watch.top_x-20)/118;
                xl = watch.top_x-slopeL*(118-y);
    //            slopeL=(lineinfo[watch.watch_lost].left-lineinfo[100].left)/(watch.watch_lost-100);
    //            xl = lineinfo[100].left+(y-100)*slopeL;
           }
           else if (watch.InLoop == 5
                   &&watch.OutLoopAngle2!=120
                   &&y < watch.OutLoopAngle2
                   && watch.zebra_flag == 0)
           {// 封住入环口，基本跟上面一样，为了鲁棒大圆环
               // xl = lineinfo[y].right - 132 + y;
               slopeL=(float)(r_border[20]-r_border[80])/60;
               watch.top_x=r_border[20]-98*slopeL;
               slopeL=(float)(watch.top_x-l_border[watch.OutLoopAngle2+1])/(117-watch.OutLoopAngle2);
               xl=watch.top_x-slopeL*(118-y);
    //            slopeL=(lineinfo[watch.watch_lost].left-lineinfo[100].left)/(watch.watch_lost-100);
    //            xl = lineinfo[100].left+(y-100)*slopeL;
           }
        //对补线后的结果进行逆透视变换
        //persp_task(xl,xr,y);

        l_border[y]=xl;
        r_border[y]=xr;
    }
}
// 默认实现：按契约 original 已是 0/255；默认直接拷贝到 imo。
// 可选：定义 SANITIZE_INPUT 时，对非 0/255 的输入做阈值归一化。
void process_original_to_imo(const uint8_t * RESTRICT original,
                             uint8_t * RESTRICT imo_out,
                             int width,
                             int height) {
    if (width <= 0 || height <= 0) return;

    // 将输入 original 拷贝到全局 Grayscale（image_process 使用该缓冲作为输入）
    for (int y = 0; y < height; ++y) {
        memcpy(Grayscale[y], original + y * width, (size_t)width);
    }

    //我的屎
    /*
    left_ring_first_angle();
    left_ring_circular_arc();
    left_ring_second_angle();
    left_ring_begin_turn();
    */
    left_ring_prepare_out();
    left_ring_out_angle();
    // 调用你的流水线
    image_process();

    // 若调用者传入的 imo_out 不是全局 imo，则把结果复制回去
    if (imo_out != &imo[0][0]) {
        for (int y = 0; y < height; ++y) {
            memcpy(imo_out + y * width, &imo[y][0], (size_t)width);
        }
    }
}
//！！接口文件！！
