#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x0D, 0xB7, 0x36, 0xD9, 0x13, 0x0A, 0x40, 0x6B, 0x9F, 0xA6, 0xB1, 0x18, 0x35, 0xC9, 0x8E, 0x02 }
PBL_APP_INFO(MY_UUID,
             "Dutch Fuzzy Time", "Maik Joosten",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

static Window window;
static GFont font_small;
static GFont font_big;
static InverterLayer inverter;
static PropertyAnimation inverter_anim;

typedef struct
{
        TextLayer layer;
        PropertyAnimation anim;
        const char * text;
        const char * old_text;
} word_t;

static word_t first_word;
static word_t first_word_between;
static word_t second_word;
static word_t second_word_between;
static word_t third_word;


static const char *hours[] = {
	"twaalf",
	"een",
	"twee",
	"drie",
	"vier",
	"vijf",
	"zes",
	"zeven",
	"acht",
	"negen",
	"tien",
	"elf",
	"twaalf"
};
static const char *minutes[] = {
	"",
	"",
	"",
	"",
	"",
	"vijf",
	"",
	"",
	"",
	"",
	"tien"
};


void
text_layer_setup(
        Window * window,
        TextLayer * layer,
        GRect frame,
        GFont font
)
{
        text_layer_init(layer, frame);
        text_layer_set_text(layer, "");
        text_layer_set_text_color(layer, GColorWhite);
        text_layer_set_background_color(layer, GColorClear);
     	text_layer_set_text_alignment(layer, GTextAlignmentCenter);
        text_layer_set_font(layer, font);
        layer_add_child(&window->layer, &layer->layer);
}

static const char *
min_string(
        int i
)
{
        return minutes[i];
}


static const char *
hour_string(
        int h
)
{
        if (h < 12)
                return hours[h];
        else
                return hours[h - 12];
}


static void
update_word(
        word_t * const word
)
{
        text_layer_set_text(&word->layer, word->text);
        if (word->text != word->old_text)
                animation_schedule(&word->anim.animation);
}



static void
nederlands_format(
        int h,
        int m
)
{
		first_word.text = "";
		first_word_between.text = "";
		second_word.text = "";
		second_word_between.text = "";
		third_word.text = "";
	
		int unrounded = m;
		float temp_m = m;
		temp_m = temp_m / 5;
		m = temp_m + 0.5;
		m = m * 5;
	
		if(m == 0 || m == 60) {
			if(m == 60) {
				h++;
			}
			first_word_between.text = hour_string(h);
			second_word_between.text = "uur";
		} else if(m == 30) {
			first_word_between.text = "half";
			second_word_between.text = hour_string(h + 1);
		} else if(m == 15) {
			first_word.text = "kwart";
			second_word.text = "over";
			third_word.text = hour_string(h);
		} else if(m == 45) {
			first_word.text = "kwart";
			second_word.text = "voor";
			third_word.text = hour_string(h + 1);
		} else if(m > 30) {
			if(m < 45) {
				first_word.text = min_string(m - 30);
				second_word.text = "over half";
				third_word.text = hour_string(h + 1);
			} else {
				first_word.text = min_string(60 - m);
				second_word.text = "voor";
				third_word.text = hour_string(h + 1);
			}
		} else {
			if(m > 15) {
				first_word.text = min_string(30 - m);
				second_word.text = "voor half";
				third_word.text = hour_string(h + 1);
			} else {
				first_word.text = min_string(m);
				second_word.text = "over";
				third_word.text = hour_string(h);
			}
		}
	
		GRect frame;
		GRect frame_right;
		switch(unrounded - m) {
			case -2:
				frame = GRect(108, 166, 36, 1);
				frame_right = frame;
				frame_right.origin.x = 0;
				break;
			case -1:
				frame = GRect(0, 166, 36, 1);
				frame_right = frame;
				frame_right.origin.x = 36;
				break;
			case 0:
				frame = GRect(36, 166, 36, 1);
				frame_right = frame;
				frame_right.origin.x = 63;
				frame_right.size.w = 18;
				break;
			case 1:
				frame = GRect(63, 166, 36, 1);
				frame_right = frame;
				frame_right.origin.x = 72;
				frame_right.size.w = 36;
				break;
			case 2:
				frame = GRect(72, 166, 36, 1);
				frame_right = frame;
				frame_right.origin.x = 108;
				break;
		}
		property_animation_init_layer_frame(&inverter_anim, (Layer *)&inverter, &frame, &frame_right);
		animation_schedule(&inverter_anim.animation);

}

/** Called once per minute */
static void
handle_tick(
        AppContextRef ctx,
        PebbleTickEvent * const event
)
{
        (void) ctx;
        const PblTm * const ptm = event->tick_time;

        int hour = ptm->tm_hour;
        int min = ptm->tm_min;
	
		first_word.old_text = first_word.text;
		first_word_between.old_text = first_word_between.text;
		second_word.old_text = second_word.text;
        second_word_between.old_text = second_word_between.text;
        third_word.old_text = third_word.text;

        nederlands_format(hour,  min);

        update_word(&first_word);
        update_word(&first_word_between);
        update_word(&second_word);
        update_word(&second_word_between);
        update_word(&third_word);
}


static void
text_layer(
        word_t * word,
        GRect frame,
        GFont font
)
{
        text_layer_setup(&window, &word->layer, frame, font);

        GRect frame_right = frame;
        frame_right.origin.x = 150;

        property_animation_init_layer_frame(
                &word->anim,
                &word->layer.layer,
                &frame_right,
                &frame
        );

        animation_set_duration(&word->anim.animation, 500);
        animation_set_curve(&word->anim.animation, AnimationCurveEaseIn);
	
		animation_set_duration(&inverter_anim.animation, 2000);
        animation_set_curve(&inverter_anim.animation, AnimationCurveEaseIn);
}


static void
handle_init(
        AppContextRef ctx
)
{
        (void) ctx;

        window_init(&window, "Main");
        window_stack_push(&window, true);
        window_set_background_color(&window, GColorBlack);

        resource_init_current_app(&RESOURCES);

        font_big = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_40));
        font_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_30));

        text_layer(&first_word, GRect(0, 12, 143, 48), font_big);
        text_layer(&second_word, GRect(0, 62, 143, 42), font_small);
        text_layer(&third_word, GRect(0, 96, 143, 48), font_big);
	
	    text_layer(&first_word_between, GRect(0, 27, 143, 48), font_big);
	    text_layer(&second_word_between, GRect(0, 83, 143, 48), font_big);
	
		inverter_layer_init(&inverter, GRect(0, 166, 36, 1));
		layer_add_child(&window.layer, (Layer *)&inverter);

}


static void
handle_deinit(
        AppContextRef ctx
)
{
        (void) ctx;

        fonts_unload_custom_font(font_small);
        fonts_unload_custom_font(font_big);
}


void
pbl_main(
        void * const params
)
{
        PebbleAppHandlers handlers = {
                .init_handler   = &handle_init,
                .deinit_handler = &handle_deinit,
                .tick_info      = {
                        .tick_handler = &handle_tick,
                        .tick_units = MINUTE_UNIT,
                },
        };

        app_event_loop(params, &handlers);
}