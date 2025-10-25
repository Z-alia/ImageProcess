#include "processor.h"
#include "global_image_buffer.h"
#include <string.h>
#include <stddef.h>
#include "image.h"
#include "dynamic_log.h"k
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
void left_ring_confirm();
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
            //&& y < watch.InLoopAngleL
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
//*函数名称：void left_ring_circular_arc()
/*功能说明：左环上凸弧扫描函数
*/
void left_ring_circular_arc()
{
    if (watch.InLoop != 1&&watch.InLoop != 2)return;//在循环之前跳出，节省时间
    //beep(20);
    for(int y=loop_forward_near;y<loop_forward_far;y++)//逐行扫描
    {
        if (//y <watch.InLoopAngle2  &&
               (watch.InLoopAngleL<65)//去除了两个积分条件
           &&(y>(watch.InLoopAngleL+20))
           //&&y <watch.InLoopCirc   //初始化给120
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
    left_ring_first_angle();
    left_ring_circular_arc();
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
