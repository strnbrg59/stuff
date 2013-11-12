package net.trhj.androidflashcards;

import android.app.Activity;
import android.app.AlertDialog;
import android.widget.EditText;
import android.widget.Toast;
import android.widget.LinearLayout;
import android.content.*;
import android.util.Log;
import java.util.*;

class EditTextPopup
{
    AlertDialog alert_;

    public EditTextPopup(final Activity activity, String msg,
                         final LinkedList<String> existing_languages,
                         final PopupFunctor functor)
    {
        AlertDialog.Builder builder = new AlertDialog.Builder(activity);

        builder.setTitle("Type some text");
        builder.setMessage(msg);

        final EditText input = new EditText(activity);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.MATCH_PARENT);
        input.setLayoutParams(lp);
        builder.setView(input);

        builder.setPositiveButton(
            "OK", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                Log.w("Cardation", "New language = " + input.getText());
                if (!validInput(input.getText().toString(),
                                activity, existing_languages)) {
                    // After lots of effort, I haven't been able to figure out
                    // how to keep the dialog open.  So if the input is invalid,
                    // let's just crash.  Unfortunately, this crash happens
                    // before there's time to see the Toast.  I've tried
                    // Thread.sleep, and tried spinning, but always the Toast
                    // message is lost, dammit.
//                    throw new RuntimeException("This is a crash");
                } else {
                    functor.doit(activity, input.getText().toString());
                    dialog.dismiss();
                }
            }
        });
        builder.setNegativeButton(
            "Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
            }
        });

        alert_ = builder.create();
    }

    @SuppressWarnings("unchecked")
    boolean validInput(String str, Activity activity,
                       LinkedList<String> existing_languages) {

        // Empty string
        if (str.equals("")) {
            Toast.makeText(activity, "Empty language -- invalid",
                           Toast.LENGTH_LONG).show();
            return false;
        }
        
        // Duplicate
        LinkedList<String> languages_copy =
            (LinkedList<String>)existing_languages.clone();
        while (languages_copy.size() > 0) {
            if (str.equals(languages_copy.poll())) {
                Toast.makeText(activity,
                               "Duplicate language -- invalid",
                               Toast.LENGTH_LONG).show();
                return false;                
            }
        }

        // Non-alphanumeric
        if (!str.matches("[A-Za-z0-9]*")) {
            Toast.makeText(activity,
                           "Nonalphanumeric -- invalid",
                           Toast.LENGTH_LONG).show();
            return false;                            
        }

        return true;
    }

    public void show()
    {
        alert_.show();
    }
}
