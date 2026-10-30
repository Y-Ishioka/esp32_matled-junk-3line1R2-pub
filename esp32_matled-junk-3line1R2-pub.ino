#include <dummy.h>

/*
 *  Copyright(C) by Yukiya Ishioka
 */

#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>     /* RTOS task related API prototypes. */
#include <freertos/timers.h>
#include <freertos/event_groups.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

/******************************************************/
/* Wi-Fiアクセスポイントの SSID とパスフレーズ        */
/* 利用する環境のSSIDとパスフレーズを設定してください */
/******************************************************/
const char* ssid1     = "xxxxxxxxxx";
const char* password1 = "xxxxxxxxxx";
const char* ssid2     = "xxxxxxxxxx";
const char* password2 = "xxxxxxxxxx";
const char* ssid3     = "xxxxxxxxxx";
const char* password3 = "xxxxxxxxxx";
/******************************************************/

#define DEF_MSG_UPDATE      (1000 * 60 * 20)
#define DEF_TIM_UPDATE      100
#define DEF_MSG_NUM_MAX     15

// https://news.yahoo.co.jp/rss
// https://news.ntv.co.jp/rss/index.rdf
// https://rss.wor.jp/
// https://www.asahi.com/information/service/rss.html
// https://www.nhk.or.jp/toppage/rss/index.html

const char *url[] = {
  "https://pc.watch.impress.co.jp/data/rss/1.0/pcw/feed.rdf",
  "https://news.yahoo.co.jp/rss/topics/top-picks.xml",
  "https://news.web.nhk/n-data/conf/na/rss/cat0.xml",
  "https://www.asahi.com/rss/asahi/newsheadlines.rdf",
  "https://assets.wor.jp/rss/rdf/sankei/flash.rdf",
  "https://assets.wor.jp/rss/rdf/yomiuri/latestnews.rdf",
  "https://assets.wor.jp/rss/rdf/nikkei/news.rdf",
  "https://assets.wor.jp/rss/rdf/ynnews/news.rdf",

  "https://news.yahoo.co.jp/rss/topics/domestic.xml",
  "https://news.yahoo.co.jp/rss/topics/it.xml",
  "https://news.yahoo.co.jp/rss/topics/science.xml",
  "https://news.yahoo.co.jp/rss/topics/world.xml",
  "https://www.asahi.com/rss/asahi/politics.rdf",
  "https://assets.wor.jp/rss/rdf/yomiuri/culture.rdf",
  "https://assets.wor.jp/rss/rdf/yomiuri/science.rdf",
  NULL  /* 終了マーク */
};


#define BTNPIN     4
#define VOLPIN    34
#define LEDPIN     2

#define SIN1A     12
#define SIN2A     13
#define SIN3A     14
#define CLKPIN    15
#define LATPIN    16
#define SIN1B     17
#define SIN2B     18
#define SIN3B     19
#define SIN1C     25
#define SIN2C     26
#define SIN3C     27

#define DEF_BRIGHT_TIME   20
#define DEF_OFF_TIME      2

unsigned char  msg_test[] = "システム初期化中";

extern  const unsigned char  SHNHN16_FNT[];
extern  const unsigned char  jiskan16_fnt[];
extern  const unsigned char  Utf8Sjis_tbl[];

#define DEF_FONT_SIZE        16
#define DEF_FONT_A16_VAR     SHNHN16_FNT
#define DEF_FONT_K16_VAR     jiskan16_fnt

#define DEF_LED_WIDTH        16
#define DEF_LED_HIGHT        16
#define DEF_MOD_WIDTH        (DEF_LED_WIDTH * 2)
#define DEF_MOD_NUM          4
#define DEF_DISP_WIDTH       (DEF_MOD_WIDTH * DEF_MOD_NUM)

#define DEF_MSG1CNT_COL      10
#define DEF_MSG1POS_COL      5

#define DEF_OUTDATA_NUM      3000
#define DEF_CHAR_NUM         ((DEF_OUTDATA_NUM-DEF_DISP_WIDTH*2)/8)

unsigned char  disp_buff_a[ DEF_LED_HIGHT ][ DEF_DISP_WIDTH ];
unsigned char  disp_buff_b[ DEF_LED_HIGHT ][ DEF_DISP_WIDTH ];
int  msg1_col[ 2 * 2 * DEF_MOD_NUM ];
unsigned char  msg1_col_dec[ 2 * 2 * DEF_MOD_NUM + 20 ];
unsigned char  msg2_col_dec[ 2 * 2 * DEF_MOD_NUM + 20 ];

unsigned char  dat_buffer[ DEF_FONT_SIZE ][ DEF_OUTDATA_NUM ];
unsigned char  adr_tbl[ DEF_LED_HIGHT ][ DEF_LED_WIDTH ];
unsigned char  msg_buffer[ DEF_MSG_NUM_MAX ][ DEF_OUTDATA_NUM / 8 * 2 + 1 ];
int  led_msg_buf_len[ DEF_MSG_NUM_MAX ];
int  led_msg_len ;
int  led_pos;
int  msg_chg_flag;
int  disp_slide;
int  def_slide_cnt;
int  time_flag;

TaskHandle_t  thand_mainTask;
TaskHandle_t  thand_slideTask;
TaskHandle_t  thand_wifiTask;
TimerHandle_t thand_timeMsg;
TimerHandle_t thand_timeTime;
EventGroupHandle_t  ehandMsg;

const char  *def_week_msg[] = {
 "(日)", "(月)", "(火)", "(水)", "(木)", "(金)", "(土)"
};

const char  *def_number_msg[] = {
 "０", "１", "２", "３", "４", "５", "６", "７", "８", "９"
};

/*************************************************
 *  
 *************************************************/
static volatile  int  usec_cnt;

void  usec_delay( int usec )
{
    int  cnt;

    if( usec <= 1 ) {
        usec = 1;
    }
    cnt = usec;

    for( usec_cnt=0; usec_cnt< cnt ; usec_cnt++ ) ;
}


/*************************************************
 *  
 *************************************************/
void  dev_led_clr_out( void )
{
    int  tmpcol;
    int  num;

  for( num=0 ; num<DEF_MOD_NUM ; num++ ) {
    for( tmpcol=0 ; tmpcol<DEF_LED_WIDTH ; tmpcol++ ) {
        digitalWrite( SIN1A, adr_tbl[0][tmpcol] );
        digitalWrite( SIN1B, adr_tbl[0][tmpcol] );
        digitalWrite( SIN1C, adr_tbl[0][tmpcol] );

        digitalWrite( SIN2A, 0 );
        digitalWrite( SIN3A, 0 );
        digitalWrite( SIN2B, 0 );
        digitalWrite( SIN3B, 0 );
        digitalWrite( SIN2C, 0 );
        digitalWrite( SIN3C, 0 );

        usec_delay( 1 );
        digitalWrite( CLKPIN, 1 );
        usec_delay( 1 );
        digitalWrite( CLKPIN, 0 );
    }
  }
    digitalWrite( LATPIN, 0 );
    usec_delay( 1 );
    digitalWrite( LATPIN, 1 );
}


/*************************************************
 *  表示データ配列のクリア
 *************************************************/
void  clear_message( unsigned char *buff, int width )
{
    int  i, j ;

    for( i=0 ; i<DEF_FONT_SIZE ; i++ ) {
        for( j=0 ; j<width ; j++ ) {
            *buff++ = 0x00;
        }
    }
}


/*************************************************
 *  set font data to display buffer
 *************************************************/
void  set_font( unsigned char *font, unsigned char *buff, int pos, int width )
{
    int  i, j, k;
    int  row;
    int  w = (width/8);   /* font width byte */
    unsigned char  pat;

    /* row */
    for( i=0 ; i<DEF_FONT_SIZE ; i++ ) {
        row = DEF_OUTDATA_NUM * i;
        /* col */
        for( j=0 ; j<w ; j++ ) {
            pat = 0x80;
            for( k=0 ; k<8 ; k++ ) {
                if( (font[ i * w + j ] & pat) != 0 ) {
                    /*         base up/low  offset */
                    buff[ row + pos + j*8 + k ] = 1;
                }
                pat >>= 1; /* bit shift */
            }
        }
    }
}


/*************************************************
 *  １文字分のフォントデータを表示データ配列の指定位置へセット
 *************************************************/
void  set_font_v2( unsigned char *font, unsigned char *buff, int pos, int width, int outwd )
{
    int  i, j, k;
    int  row;
    int  w = (width/8);   /* font width byte */
    unsigned char  pat;

    /* row */
    for( i=0 ; i<DEF_FONT_SIZE ; i++ ) {
        row = outwd * i;
        /* col */
        for( j=0 ; j<w ; j++ ) {
            pat = 0x80;
            for( k=0 ; k<8 ; k++ ) {
                if( (font[ i * w + j ] & pat) != 0 ) {
                    /*         base up/low  offset */
                    buff[ row + pos + j*8 + k ] = 1;
                }
                pat >>= 1; /* bit shift */
            }
        }
    }
}


/*************************************************
 *  
 *************************************************/
void UTF8_To_SJIS_cnv(unsigned char utf8_1, unsigned char utf8_2, unsigned char utf8_3, unsigned int* spiffs_addrs)
{
  unsigned int  UTF8uint = utf8_1*256*256 + utf8_2*256 + utf8_3;
   
  if(utf8_1>=0xC2 && utf8_1<=0xD1){
    *spiffs_addrs = ((utf8_1*256 + utf8_2)-0xC2A2)*2 + 0xB0;
  }else if(utf8_1==0xE2 && utf8_2>=0x80){
    *spiffs_addrs = (UTF8uint-0xE28090)*2 + 0x1EEC;
  }else if(utf8_1==0xE3 && utf8_2>=0x80){
    *spiffs_addrs = (UTF8uint-0xE38080)*2 + 0x9DCC;
  }else if(utf8_1==0xE4 && utf8_2>=0x80){
    *spiffs_addrs = (UTF8uint-0xE4B880)*2 + 0x11CCC;
  }else if(utf8_1==0xE5 && utf8_2>=0x80){
    *spiffs_addrs = (UTF8uint-0xE58085)*2 + 0x12BCC;
  }else if(utf8_1==0xE6 && utf8_2>=0x80){
    *spiffs_addrs = (UTF8uint-0xE6808E)*2 + 0x1AAC2;
  }else if(utf8_1==0xE7 && utf8_2>=0x80){
    *spiffs_addrs = (UTF8uint-0xE78081)*2 + 0x229A6;
  }else if(utf8_1==0xE8 && utf8_2>=0x80){
    *spiffs_addrs = (UTF8uint-0xE88080)*2 + 0x2A8A4;
  }else if(utf8_1==0xE9 && utf8_2>=0x80){
    *spiffs_addrs = (UTF8uint-0xE98080)*2 + 0x327A4;
  }else if(utf8_1>=0xEF && utf8_2>=0xBC){
    *spiffs_addrs = (UTF8uint-0xEFBC81)*2 + 0x3A6A4;
    if(utf8_1==0xEF && utf8_2==0xBD && utf8_3==0x9E){
      *spiffs_addrs = 0x3A8DE;
    }
  }
}


/*************************************************
 *  
 *************************************************/
int  utf8_to_sjis( unsigned char *buff, unsigned char *code1, unsigned char *code2 )
{
  unsigned char  utf8_1, utf8_2, utf8_3;
  unsigned char  sjis[2];
  unsigned int  sp_addres;
  int  pos = 0;
  int fnt_cnt = 0;
 
  if(buff[pos]>=0xC2 && buff[pos]<=0xD1){
    /* UTF8 2bytes */
    utf8_1 = buff[pos];
    utf8_2 = buff[pos+1];
    utf8_3 = 0x00;
    fnt_cnt = 2;
  }else if(buff[pos]>=0xE2 && buff[pos]<=0xEF){
    /* UTF8 3bytes */
    utf8_1 = buff[pos];
    utf8_2 = buff[pos+1];
    utf8_3 = buff[pos+2];
    fnt_cnt = 3;
  }else{
    utf8_1 = buff[pos];
    utf8_2 = 0x00;
    utf8_3 = 0x00;
    fnt_cnt = 1;
  }

  /* UTF8 to Sjis change table position */
  UTF8_To_SJIS_cnv( utf8_1, utf8_2, utf8_3, &sp_addres );
  *code1 = Utf8Sjis_tbl[ sp_addres   ];
  *code2 = Utf8Sjis_tbl[ sp_addres+1 ];

  return  fnt_cnt;
}


/*************************************************
 *  
 *************************************************/
unsigned char  *get_fontx2_a( unsigned char *font, unsigned int code )
{
    unsigned char  *address = NULL ;
    unsigned int  fontbyte ;

    fontbyte = (font[14] + 7) / 8 * font[15] ;
    address = &font[17] + fontbyte * code ;

    return  address ;
}


/*************************************************
 *  
 *************************************************/
unsigned char  *get_fontx2_k( unsigned char *font, unsigned int code )
{
    unsigned char  *address = NULL ;
    unsigned char  *tmp ;
    unsigned int  blknum, i, fontnum ;
    unsigned int  bstart, bend ;
    unsigned int  fontbyte ;

    fontbyte = (font[14] + 7) / 8 * font[15] ;
    fontnum = 0 ;

    blknum = (unsigned int)font[17] * 4 ;
    tmp = &font[18] ;
    for( i=0 ; i<blknum ; i+=4 ) {
        bstart = tmp[i]   + ((unsigned int)tmp[i+1] << 8) ;
        bend   = tmp[i+2] + ((unsigned int)tmp[i+3] << 8) ;
        if( code >= bstart && code <= bend ) {
            address = tmp + (fontnum + (code - bstart)) * fontbyte + blknum ;
            break ;
        }

        fontnum += (bend - bstart) + 1 ;
    }

    if( address == 0 ) {
        Serial.printf( "*get_fontx2_k( 0x%X ) : address = %X \n", code, address );
        if( code == 0x8780 ) {
            code = 0x8168;
        } else if( code == 0x8781 ) {
            code = 0x8168;
        } else {
            code = 0x8145; /* if there is no data, "・" */
        }
        address = get_fontx2_k( font, code );
    }

    return  address ;
}


/*************************************************
 *  for UTF-8
 *************************************************/
int  make_message( unsigned char *strbuff, unsigned int size )
{
    int  pos, bitpos;
    int  num;
    unsigned char  *fontdata;
    unsigned int  code;
    unsigned char  code1, code2 ;

    clear_message( (unsigned char *)dat_buffer, DEF_OUTDATA_NUM );

    pos = 0 ;
    bitpos = DEF_DISP_WIDTH ;
    while( pos < size ) {
        code = strbuff[ pos ] ;  /* get 1st byte */
        if( code < 0x80 ) {
            /* for ASCII code */
            if( code == 0x0d || code == 0x0a ) {
                code = ' ' ;
            }
            fontdata = get_fontx2_a( (unsigned char *)DEF_FONT_A16_VAR, code );
            set_font( fontdata, (unsigned char *)dat_buffer, bitpos, DEF_FONT_SIZE/2 );
            bitpos += DEF_FONT_SIZE/2 ;
            pos++ ;
        } else {
            /* for KANJI code */
            num = utf8_to_sjis( &strbuff[ pos ], &code1, &code2 );

            code = (code1<<8) + code2 ;  /* get 2nd byte and marge */
            if( code >= 0xa100 && code <= 0xdf00 ) {
                code = 0x8140;
            }
            fontdata = get_fontx2_k( (unsigned char *)DEF_FONT_K16_VAR, code );
            set_font( fontdata, (unsigned char *)dat_buffer, bitpos, DEF_FONT_SIZE );
            bitpos += DEF_FONT_SIZE;
            pos += num;
        }

        if( bitpos >= (DEF_OUTDATA_NUM - DEF_FONT_SIZE - DEF_DISP_WIDTH) ) {
            break ;
        }
    }

    return  bitpos + DEF_DISP_WIDTH ;
}


/*************************************************
 *  UTF-8 表示メッセージ文字列から表示データ配列の作成
 *************************************************/
int  make_message_v2( unsigned char *strbuff, unsigned int size, int xpos, unsigned char *outbuff, int outwd )
{
    int  pos, bitpos;
    int  num;
    unsigned char  *fontdata;
    unsigned int  code;
    unsigned char  code1, code2 ;

    pos = xpos;
    bitpos = 0;    /* 表示先頭のブランクのためのオフセット */
    while( pos < size ) {
        /* 表示メッセージ 1byteを取り出し */
        code = strbuff[ pos ] ;  /* get 1st byte */
        /* 半角か全角かの判定 */
        if( code < 0x80 ) {
            /* 半角の処理 */
            if( bitpos > (outwd - DEF_FONT_SIZE/2) ) {
               /* 次に全角フォントが置けなくなったら作成処理終了 */
               break ;
            }
            if( code == 0x0d || code == 0x0a ) {
                /* 改行コードはスペースへ変換 */
                code = ' ' ;
            }
            fontdata = get_fontx2_a( (unsigned char *)DEF_FONT_A16_VAR, code );
            set_font_v2( fontdata, outbuff, bitpos, DEF_FONT_SIZE/2, outwd );
            bitpos += DEF_FONT_SIZE/2 ;
            pos++ ;
        } else {
            /* 全角の処理 */
            if( bitpos > (outwd - DEF_FONT_SIZE) ) {
               /* 次に全角フォントが置けなくなったら作成処理終了 */
               break ;
            }
            num = utf8_to_sjis( &strbuff[ pos ], &code1, &code2 );

            code = (code1<<8) + code2 ;  /* get 2nd byte and marge */
            if( code >= 0xa100 && code <= 0xdf00 ) {
                code = 0x8140;
            }
            code = (code1<<8) + code2 ;  /* get 2nd byte and marge */
            fontdata = get_fontx2_k( (unsigned char *)DEF_FONT_K16_VAR, code );
            set_font_v2( fontdata, outbuff, bitpos, DEF_FONT_SIZE, outwd );
            bitpos += DEF_FONT_SIZE;
            pos += num;
        }
    }

    return  bitpos;
}


/*************************************************
 *  for S-JIS
 *************************************************/
int  make_message_sjis( unsigned char *strbuff, unsigned int size, int xpos, unsigned char *outbuff, int outwd )
{
    int  pos, bitpos;
    int  num;
    unsigned char  *fontdata;
    unsigned int  code;
    unsigned char  code1, code2 ;

    pos = 0;
    bitpos = xpos;
    while( pos < size ) {
        /* 表示メッセージ 1byteを取り出し */
        code = strbuff[ pos ];  /* get 1st byte */
        /* 半角か全角かの判定 */
        if( code < 0x80 ) {
            /* 半角の処理 */
            if( bitpos > (outwd - DEF_FONT_SIZE/2) ) {
               /* 次に全角フォントが置けなくなったら作成処理終了 */
               break;
            }
            /* esc code check */
            if( code == '&' ) {
                if( strbuff[ pos+1 ] == 'a' && strbuff[ pos+2 ] == 'm'
                 && strbuff[ pos+3 ] == 'p' && strbuff[ pos+4 ] == ';' ) {
                  pos += 4;
                }
            }
            if( code == 0x0d || code == 0x0a ) {
                /* 改行コードはスペースへ変換 */
                code = ' ';
            }
            fontdata = get_fontx2_a( (unsigned char *)DEF_FONT_A16_VAR, code );
            set_font_v2( fontdata, outbuff, bitpos, DEF_FONT_SIZE/2, outwd );
            bitpos += DEF_FONT_SIZE/2;
            pos++;
        } else {
            /* 全角の処理 */
            if( bitpos > (outwd - DEF_FONT_SIZE) ) {
               /* 次に全角フォントが置けなくなったら作成処理終了 */
               break ;
            }
            num = 2;
            code1 = (unsigned char)code;
            code2 = strbuff[ pos+1 ];
            code = (code1<<8) + code2;  /* get 2nd byte and marge */
            if( code >= 0xa100 && code <= 0xdf00 ) {
                code = 0x8140;
            }
            code = (code1<<8) + code2 ;  /* get 2nd byte and marge */
            fontdata = get_fontx2_k( (unsigned char *)DEF_FONT_K16_VAR, code );
            set_font_v2( fontdata, outbuff, bitpos, DEF_FONT_SIZE, outwd );
            bitpos += DEF_FONT_SIZE;
            pos += num;
        }
    }

    return  bitpos;
}


/*************************************************
 *  
 *************************************************/
const char  *chks = "<title>";
const char  *chke = "</title>";

void  wifi_access( void )
{
    HTTPClient  http;
    int  httpCode;
    unsigned char  *src ;
    unsigned char  *dst ;
    unsigned char  *top;
    unsigned int  lens = strlen(chks);
    unsigned int  lene = strlen(chke);
    unsigned int  len;
    int  cnt;
    int  lim;
    int  i;
    int  mode;
    int  url_num;
    int  err_cnt = 0;
    const char *url_pnt;

    digitalWrite( LEDPIN, HIGH );

    for( url_num=0 ; url_num<DEF_MSG_NUM_MAX ; url_num++ ) {

      url_pnt = url[ url_num ];
      if( url_pnt == NULL ) {
          Serial.printf("URL List check : url_num: %d\n", url_num );
          break;
      }

retry_loop:
      if( err_cnt > 3 ) {
          err_cnt = 0;
          continue;
      }
      http.begin( url_pnt );
      httpCode = http.GET();
      Serial.printf("URL: %s\n", url_pnt );
      Serial.printf("Response: %d", httpCode);
      Serial.println();
      if (httpCode == HTTP_CODE_OK) {
        String body = http.getString();
        int get_size = http.getSize();
        Serial.printf("Response size: %d\n", get_size);
#if 0
        Serial.print("Response Body: ");
        Serial.println(body);
#endif

        src = (unsigned char *)body.c_str();
        dst = msg_buffer[url_num];
        mode = 0;
        cnt = 0;
        lim = 0;

        while( cnt < DEF_CHAR_NUM && *src != 0x00 ) {
          if( ++lim > 10000 ) {
            Serial.printf("read data limit.");
            break;
          }
          if( *src != '<' ) {
            src++;
            continue;
          }
          if( mode == 0 ) {
            if( strncmp( chks, (const char*)src, (size_t)lens ) == 0 ) {
              mode = 1;
              src += lens;
              top = src;
            } else {
              src++ ;
            }
            continue;
          }
          if( mode == 1 ) {
            if( strncmp( chke, (const char*)src, (size_t)lene ) == 0) {
              len = (unsigned long)src - (unsigned long)top;
              for( i=0 ; i<len ; i++ ) {
                *dst++ = *top++;
              }
              *dst++ = ' '; /* set space */
              *dst++ = ' '; /* set space */
              src += lene;
              cnt += len + 2;

              *dst++ = ' '; /* set space */
              *dst++ = ' '; /* set space */
              cnt += 2;

              mode = 0;
            }
          }
        }

        http.end();
        if( cnt == 0 ) {
          err_cnt++;
          goto retry_loop;
        }
        Serial.printf("led_msg_char(%d): %d\n", url_num, cnt);
        led_msg_buf_len[ url_num ] = cnt;
        *dst = 0x00;

        Serial.printf("%s\n", msg_buffer[url_num] );

      } else {
        http.end();
      }
    }
    Serial.printf("url_num: %d\n", url_num );

    digitalWrite( LEDPIN, LOW );
}


/*************************************************
 *  
 *************************************************/
void  set_timeconf( void )
{
  unsigned long m;
  struct tm timeInfo;

  m = millis();
  getLocalTime(&timeInfo);  
  Serial.printf("getLocalTime() %luMS\n",millis()-m);
  Serial.printf("%04d/%02d/%02d %02d:%02d:%02d\n",timeInfo.tm_year+1900,timeInfo.tm_mon+1,timeInfo.tm_mday,timeInfo.tm_hour,timeInfo.tm_min,timeInfo.tm_sec);

  m = millis();
  configTime(9 * 3600L, 0, "ntp.nict.jp", "time.google.com", "ntp.jst.mfeed.ad.jp");
  Serial.printf("configTime() %luMS\n",millis()-m);

  getLocalTime(&timeInfo);  
  Serial.printf("%04d/%02d/%02d %02d:%02d:%02d\n",timeInfo.tm_year+1900,timeInfo.tm_mon+1,timeInfo.tm_mday,timeInfo.tm_hour,timeInfo.tm_min,timeInfo.tm_sec);

  time_flag = 1;
}


/*************************************************
 *  
 *************************************************/
void  wifi_init( void )
{
  digitalWrite( LEDPIN, HIGH );

  WiFi.disconnect(true);
  vTaskDelay( 500 );
  WiFiMulti wifiMulti;

  /* Wi-Fiアクセスポイントへの接続 */
  wifiMulti.addAP(ssid1, password1);
  wifiMulti.addAP(ssid2, password2);
  wifiMulti.addAP(ssid3, password3);
  while(wifiMulti.run() != WL_CONNECTED) {
    vTaskDelay( 100 );
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  set_timeconf();

  digitalWrite( LEDPIN, LOW );
}


/*************************************************
 *  
 *************************************************/
void  led_main( void *param )
{
  int  url_num;

  Serial.println( "call mainTask()" );

  msg_chg_flag = 0;

  clear_message( (unsigned char *)dat_buffer, DEF_OUTDATA_NUM );

  led_msg_len = make_message( (unsigned char *)msg_test, sizeof(msg_test) );
  Serial.printf( "data byte  = %d\n", sizeof(msg_test) );
  Serial.printf( "data width = %d\n", led_msg_len );
  xEventGroupWaitBits( ehandMsg, 0x03, pdTRUE, pdFALSE, 100000 );

  wifi_init();

  while( 1 ) {
    wifi_access();
    msg_chg_flag = 0;

    while( msg_chg_flag == 0 ) {
      for( url_num=0 ; url_num<DEF_MSG_NUM_MAX ; url_num++ ) {
        if( led_msg_buf_len[ url_num ] == 0 ) {
          continue;
        }
        led_msg_len = make_message( msg_buffer[ url_num ], led_msg_buf_len[ url_num ] );
        Serial.printf("led_msg_len(%d): %d\n", url_num, led_msg_len);
        xEventGroupClearBits( ehandMsg, 0x03 );
        led_pos = 0 ;
        xEventGroupWaitBits( ehandMsg, 0x03, pdTRUE, pdFALSE, 100000 );

        if( msg_chg_flag != 0 ) {
          break;
        }
      }
    }
  }
}


/*************************************************
 *  
 *************************************************/
void  led_slide( void *param )
{
  int  vol;
  int  i;

  Serial.println( "call slideTask()" );

  while( 1 ) {
    /* get speed volume */
    vol = analogRead( VOLPIN ) >> 7;
    if( vol < 2 ) {
      vol = 2;
    }
    vTaskDelay( vol );

    if( digitalRead( BTNPIN ) == HIGH ) {
      led_pos++ ;
      if( led_pos >= led_msg_len ) {
        led_pos = 0 ;
        xEventGroupSetBits( ehandMsg, 0x02 );
      }
    }
  }
}


/*************************************************
 *  
 *************************************************/
void  timer_message( void *param )
{
  msg_chg_flag = 1;
}


/*************************************************
 *  
 *************************************************/
void  timer_time_update( void *param )
{
  if( time_flag == 0 ) {
    return;
  }

#if 0
  struct tm  timeInfo;
  getLocalTime( &timeInfo );  

  sprintf( (char *)msg1_col_dec,
           "%04d年%02d月%02d日 %1d",
           timeInfo.tm_year+1900, timeInfo.tm_mon+1, timeInfo.tm_mday, timeInfo.tm_wday );
  sprintf( (char *)msg2_col_dec,
           "%02d:%02d:%02d",
           timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec );
#else
#if 0
  sprintf( (char *)msg1_col_dec,
           "０３月１４日(金)" );
  sprintf( (char *)msg2_col_dec,
           "１７：３５：２５" );
#else
  struct tm  timeInfo;
  getLocalTime( &timeInfo );  

  sprintf( (char *)msg1_col_dec,
           "%s%s月%s%s日%s",
           def_number_msg[(timeInfo.tm_mon+1)/10], def_number_msg[(timeInfo.tm_mon+1)%10],
           def_number_msg[timeInfo.tm_mday/10], def_number_msg[timeInfo.tm_mday%10],
           def_week_msg[timeInfo.tm_wday]  );
  sprintf( (char *)msg2_col_dec,
           "%s%s：%s%s：%s%s",
           def_number_msg[timeInfo.tm_hour/10], def_number_msg[timeInfo.tm_hour%10],
           def_number_msg[timeInfo.tm_min/10], def_number_msg[timeInfo.tm_min%10],
           def_number_msg[timeInfo.tm_sec/10], def_number_msg[timeInfo.tm_sec%10] );
#endif
#endif
  clear_message( (unsigned char *)disp_buff_a, DEF_DISP_WIDTH );
  clear_message( (unsigned char *)disp_buff_b, DEF_DISP_WIDTH );
  make_message_v2( msg1_col_dec, strlen((const char *)msg1_col_dec), 0, (unsigned char *)disp_buff_a, DEF_DISP_WIDTH );
  make_message_v2( msg2_col_dec, strlen((const char *)msg2_col_dec), 0, (unsigned char *)disp_buff_b, DEF_DISP_WIDTH );
}


/*************************************************
 *  
 *************************************************/
void setup( void )
{
  int  col, row;
  int  i, j;

  /* init serial */
  Serial.begin(115200);
  Serial.println( "call setup()" );

  pinMode( BTNPIN,  INPUT_PULLUP);
  pinMode( LEDPIN,  OUTPUT );
  digitalWrite( LEDPIN, LOW );

  pinMode( SIN1A,  OUTPUT );
  pinMode( SIN2A,  OUTPUT );
  pinMode( SIN3A,  OUTPUT );
  pinMode( CLKPIN,  OUTPUT );
  pinMode( LATPIN,  OUTPUT );
  pinMode( SIN1B,  OUTPUT );
  pinMode( SIN2B,  OUTPUT );
  pinMode( SIN3B,  OUTPUT );
  pinMode( SIN1C,  OUTPUT );
  pinMode( SIN2C,  OUTPUT );
  pinMode( SIN3C,  OUTPUT );

  digitalWrite( SIN1A, LOW );
  digitalWrite( SIN2A, LOW );
  digitalWrite( SIN3A, LOW );
  digitalWrite( CLKPIN, LOW );
  digitalWrite( LATPIN, HIGH );
  digitalWrite( SIN1B, LOW );
  digitalWrite( SIN2B, LOW );
  digitalWrite( SIN3B, LOW );
  digitalWrite( SIN1C, LOW );
  digitalWrite( SIN2C, LOW );
  digitalWrite( SIN3C, LOW );

  for( i=0 ; i<16 ; i++ ) {
    for( j=0 ; j<16 ; j++ ) {
      if( i == j ) {
        adr_tbl[i][j] = 1;
      } else {
        adr_tbl[i][j] = 0;
      }
    }
  }

  for( i=0 ; i<DEF_MSG_NUM_MAX ; i++ ) {
    led_msg_buf_len[i] = 0;
  }

  /* clear MATLED */
  dev_led_clr_out();
  clear_message( (unsigned char *)disp_buff_a, DEF_DISP_WIDTH );
  clear_message( (unsigned char *)disp_buff_b, DEF_DISP_WIDTH );

  time_flag = 0;
  led_pos = 0;

  for( i=0 ; i<2 * 2 * DEF_MOD_NUM ; i++ ) {
    msg1_col[i] = 0;
    msg1_col_dec[i] = ' ';
    msg2_col_dec[i] = ' ';
  }

  ehandMsg = xEventGroupCreate();

  /* create task */
  xTaskCreatePinnedToCore( led_main,
                           "LED_MAIN",
                           0x2000,
                           NULL,
                           10,
                           &thand_mainTask,
                           0 );

  /* create task */
  xTaskCreatePinnedToCore( led_slide,
                           "LED_SLID",
                           0x2000,
                           NULL,
                           configMAX_PRIORITIES - 1,
                           &thand_slideTask,
                           0 );

  /* create timer */
  thand_timeMsg = xTimerCreate( "TIM_MSG",
                                DEF_MSG_UPDATE,
                                pdTRUE,
                                NULL,
                                (TimerCallbackFunction_t)timer_message );

  /* create timer */
  thand_timeTime = xTimerCreate( "TIM_SEC",
                                 DEF_TIM_UPDATE,
                                 pdTRUE,
                                 NULL,
                                 (TimerCallbackFunction_t)timer_time_update );

  xTimerStart( thand_timeMsg, 0 );
  xTimerStart( thand_timeTime, 0 );
}


/*************************************************
 *  
 *************************************************/
void loop( void )
{
  int  row, col;
  int  num;
  int  hi;
  int  l_led_pos;
  int  pos1, pos2;
  unsigned char  tmpchar ;
  unsigned char *buff2 = (unsigned char *)dat_buffer ;

  l_led_pos = led_pos;
  for( hi=0 ; hi<DEF_LED_HIGHT ; hi++ ) {
    row  = DEF_OUTDATA_NUM * hi + l_led_pos;

    for( num=DEF_MOD_NUM-1 ; num>=0 ; num-- ) {
      for( col=0 ; col<DEF_LED_WIDTH ; col++ ) {

        /* for SIN1 */
        digitalWrite( SIN1A, adr_tbl[hi][col] );
        digitalWrite( SIN1B, adr_tbl[hi][col] );
        digitalWrite( SIN1C, adr_tbl[hi][col] );

        /* for SIN2 */
        pos1 = (DEF_LED_WIDTH * 2) * DEF_MOD_NUM - 1 - ((DEF_MOD_NUM - num -1) * (DEF_LED_WIDTH * 2) + col) - DEF_LED_WIDTH;
        digitalWrite( SIN2A, (uint8_t)disp_buff_a[hi][pos1] );
        pos2 = (DEF_LED_WIDTH * 2) * DEF_MOD_NUM - 1 - ((DEF_MOD_NUM - num -1) * (DEF_LED_WIDTH * 2) + col) - DEF_LED_WIDTH;
        digitalWrite( SIN2B, (uint8_t)disp_buff_b[hi][pos1] );
        if( buff2[ row + DEF_MOD_WIDTH * num + DEF_LED_WIDTH - col -1 ] ) {
          digitalWrite( SIN2C, 1 );
        } else {
          digitalWrite( SIN2C, 0 );
        }

        /* for SIN3 */
        pos1 = (DEF_LED_WIDTH * 2) * DEF_MOD_NUM - 1 - ((DEF_MOD_NUM - num -1) * (DEF_LED_WIDTH * 2) + col);
        digitalWrite( SIN3A, (uint8_t)disp_buff_a[hi][pos1] );
        pos2 = (DEF_LED_WIDTH * 2) * DEF_MOD_NUM - 1 - ((DEF_MOD_NUM - num -1) * (DEF_LED_WIDTH * 2) + col);
        digitalWrite( SIN3B, (uint8_t)disp_buff_b[hi][pos1] );
        if( buff2[ row + DEF_MOD_WIDTH * num + DEF_LED_WIDTH + DEF_LED_WIDTH - col -1 ] ) {
          digitalWrite( SIN3C, 1 );
        } else {
          digitalWrite( SIN3C, 0 );
        }

        usec_delay( 1 );
        digitalWrite( CLKPIN, 1 );
        usec_delay( 1 );
        digitalWrite( CLKPIN, 0 );
      }
    }

    digitalWrite( LATPIN, 0 );
    usec_delay( 1 );
    digitalWrite( LATPIN, 1 );
    usec_delay( DEF_BRIGHT_TIME );
  }

  dev_led_clr_out();
  usec_delay( DEF_OFF_TIME );
}

