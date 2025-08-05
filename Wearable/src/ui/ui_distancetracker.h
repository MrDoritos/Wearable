#pragma once

#include "ui.h"
#include "ui_texturewriter.h"
#include "ui_peripheral_log.h"

#include "config.h"
#include "log.h"
#include "wbl_func.h"
#include "peripheral_log.h"

#include "sprites.h"

namespace wbl {
namespace UI {

template<typename Buffer, typename ElementT = ElementBaseT<Buffer>>
struct UIDistanceTrackerT : public ElementT {
    template<typename Log, typename T, int N>
    struct FieldProvider {
        using log_type = Log;
        using time_type = typename log_type::time_type;
        using value_type = typename log_type::value_type;
        using point_type = typename log_type::point_type;
        using provider_type = LogField<LogFieldProvider<log_type, DataValueAccessor<point_type, T, N>>>;
    };

    using FPPEDST = FieldProvider<DLPEDST, uint16_t, 0>;
    using FPPEDLT = FieldProvider<DLPEDLT, uint16_t, 0>;
    using FPODOST = FieldProvider<DLCAMM8ODOST, uint32_t, 0>;
    using FPODOLT = FieldProvider<DLCAMM8ODOLT, uint32_t, 0>;
    using FPODOTOTALST = FieldProvider<DLCAMM8ODOST, uint32_t, 1>;

    struct ElementInfo : public ElementT {
        using ElementT::ElementT;
        using ElementT::operator<<;

        void on_draw(Event *event) override {
            TextureWriterT<> writer(this);

            typename FPODOTOTALST::provider_type odo(&wbl::log.camm8_odo_st);
            typename FPPEDST::provider_type ped(&wbl::log.ped_st);

            uint16_t cur_steps = ped.has_index(-1) ? ped.get_value(-1) : 0;
            uint32_t tot_dist = odo.has_index(-1) ? odo.get_value(-1) : 0;

            writer.printf(Sprites::font, "%u Steps %lum", cur_steps, tot_dist);
        }
    };

    struct ElementSplit : public ElementT {
        using ElementT::ElementT;
        using ElementT::operator<<;

        int64_t split_time;
        int64_t dp_time;

        constexpr ElementSplit(Buffer &buffer, const int64_t &split_time, const int64_t &dp_time):ElementT(buffer),split_time(split_time),dp_time(dp_time) {
            this->height = DimensionMinMax(16);

        }
        constexpr ElementSplit():ElementT(Sprites::display) {
            this->height = DimensionMinMax(16);
        }

        void on_draw(Event *event) override {
            TextureWriterT<> writer(this);

            typename FPODOLT::provider_type odo(wbl::log.camm8_odo_lt);
            typename FPPEDLT::provider_type ped(wbl::log.ped_st);

            uint16_t steps = ped.get_size() > 0 ? ped.get_binary_value(dp_time) : 0;
            uint32_t dist = odo.get_size() > 0 ? odo.get_binary_value(dp_time) : 0;

            int64_t t_milliseconds = split_time / 1000;
            int seconds = (t_milliseconds / 1000) % 60;
            int millis = t_milliseconds % 1000;
            int mins = (t_milliseconds / 60000) % 60;
            int hrs = (t_milliseconds / (60*60*1000));

            writer.printf(Sprites::font, "%u %lum ", steps, dist);

            if (hrs)
                writer.printf(Sprites::font, "%i:", hrs);
            if (mins)
                writer.printf(Sprites::font, "%02i:", mins);
            writer.printf(Sprites::font, "%02i", seconds);
            writer.printf(Sprites::minifont, ".%03i", millis);
        }
    };

    LoopBufferT<ElementSplit, 4> splits;
    ElementInfo info;

    constexpr UIDistanceTrackerT(Buffer &buffer):ElementT(buffer, "dist"),info(buffer) {
        this->append_child(&info);

        *this << StyleInfo { .width {128}, .height{96} };
        info << StyleInfo { .height{16} };

        reset_timer();
    }

    int64_t split_start = 0;
    int64_t split_timer = 0;

    enum TimerState : uint8_t {
        TRESET=1,
        TRUNNING=2,
        TSTOPPED=4,
    } timer_state;

    void add_split() {
        int64_t t = get_time();
        int64_t dp_t = timestamp_micros();

        wbl::log.pushOdometer();
        wbl::log.pushPedometer();


        //splits.push_back(ElementSplit(this->buffer, t, dp_t));
        if (splits.index >= splits.capacity())
            splits.index = 0;
        if (splits.count < splits.capacity())
            splits.count++;
        
        ElementSplit *ee = &splits.data[splits.index++];

        new (ee) ElementSplit(this->buffer, t, dp_t);

        WBL_DF("Add split %i %lli %lli\n", splits.size(), dp_t, t);

        this->child = &info;
        this->child->parent = this;

        IElement *ch = this->child;

        ch->sibling = ee;

        ch = ch->sibling;
        ch->parent = this;

        WBL_D("SET");

        for (int i = splits.size() - 2; i > -1; i--) {
            WBL_DF("%i %p\n", i, ch);
            ch->sibling = &splits.get(i);
            ch->parent = this;
            ch = ch->sibling;    
        }

        ch->sibling = nullptr;

        WBL_D("DONE");

        this->dispatch_parent(Event::CONTENT_SIZE, Event::CHANGE);
    }

    void split_time() {
        int64_t n = timestamp_micros();
        int64_t dif = n - split_start;

        split_start = n;
        split_timer += dif;
    }

    void reset_time() {
        split_start = timestamp_micros();
        split_timer = 0;
    }

    void stop_timer() {
        WBL_D("Stop timer");

        timer_state = TSTOPPED;
    }

    int64_t get_time() {
        if (timer_state & TRESET)
            return 0;

        if (timer_state & TSTOPPED)
            return split_timer;

        return (timestamp_micros() - split_start) + split_timer;
    }

    void start_timer() {
        WBL_D("Start timer");

        if (timer_state & TRESET) {
            timer_state = TRUNNING;
            reset_time();
            return;
        }

        split_time();

        timer_state = TRUNNING;
    }
    
    void reset_timer() {
        WBL_D("Reset timer");

        timer_state = TRESET;
    }

    void on_user_input(Event *event) override {
        EventValues val = event->value;
        if (val & EventValues::DPAD_UP || val & EventValues::DPAD_ENTER) {
            event->stopDefault();
        }
        if (val & EventValues::PRESSED) {
            if (val & EventValues::DPAD_UP) {
                if (timer_state & TRUNNING)
                    stop_timer();
                else
                if (timer_state & TSTOPPED)
                    start_timer();
                else
                if (timer_state & TRESET)
                    start_timer();
            }
            if (val & EventValues::DPAD_ENTER) {
                if (timer_state & TRUNNING)
                    add_split();
                else
                if (timer_state & TSTOPPED)
                    reset_timer();
                else
                if (timer_state & TRESET)
                    start_timer();
            }
        }
    }
};


}
}