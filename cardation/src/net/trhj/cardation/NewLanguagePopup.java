package net.trhj.cardation;

import android.app.Activity;
import android.app.AlertDialog;
import android.widget.EditText;
import android.widget.Toast;
import android.widget.LinearLayout;
import android.content.*;
import android.util.Log;
import java.util.*;

class NewLanguagePopup
{
    AlertDialog alert_;

    public NewLanguagePopup(final Activity activity, String msg,
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
                    functor.onNegativeButton(activity);
                    dialog.dismiss();
                } else {
                    functor.onPositiveButton(activity,
                                             input.getText().toString());
                    dialog.dismiss();
                }
            }
        });
        builder.setNegativeButton(
            "Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
                functor.onNegativeButton(activity);
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
