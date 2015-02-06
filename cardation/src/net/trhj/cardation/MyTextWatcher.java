package net.trhj.cardation;

import android.text.*;
import android.widget.Button;
import android.widget.EditText;

// Inheriting from this we can avoid typing empty implementations of all the
// mandatory functions that we don't care about.
public class MyTextWatcher implements TextWatcher
{
    EditText widget_;
    Button save_button_;

    public MyTextWatcher(EditText widget, Button save_button) {
        widget_ = widget;
        save_button_ = save_button;
    }

    @Override
    public void onTextChanged(CharSequence s, int start, int before,
                              int count) {
        if (widget_.getText().length() > 0) {
            save_button_.setEnabled(true);
        }
    }

    @Override
    public void afterTextChanged(Editable arg0) {}
    @Override
    public void beforeTextChanged(CharSequence s, int strt, int cnt, int ftr) {}
}

