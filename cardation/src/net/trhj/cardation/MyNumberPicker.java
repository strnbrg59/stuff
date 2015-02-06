package net.trhj.cardation;

import android.widget.TextView;

// Can't use MyNumberPicker as that starts with Android 3.x.  So making
// one by hand.
public class MyNumberPicker {

    int min_val_;
    int max_val_;
    TextView widget_;
    int curr_val_;

    public MyNumberPicker(int min_val, int max_val, int init_val,
                        TextView widget)
    {
        min_val_ = min_val;
        max_val_ = max_val;
        curr_val_ = init_val;
        widget_ = widget;

        widget_.setText(Integer.toString(curr_val_));
    }

    public void up()
    {
        if (curr_val_ != max_val_) {
            curr_val_ ++;
            widget_.setText(Integer.toString(curr_val_));
        }
    }

    public void down()
    {
        if (curr_val_ != min_val_) {
            curr_val_ --;
            widget_.setText(Integer.toString(curr_val_));
        }
    }

    public int getVal()
    {
        return curr_val_;
    }
}

