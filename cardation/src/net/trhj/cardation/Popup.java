package net.trhj.cardation;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.*;

class Popup
{
    AlertDialog alert_;

    public Popup(final Activity activity, String msg,
                 final PopupFunctor functor)
    {
        AlertDialog.Builder builder = new AlertDialog.Builder(activity);
        builder.setTitle("Confirm");
        builder.setMessage(msg);
        builder.setPositiveButton(
            "YES", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                functor.onPositiveButton(activity, null);
                dialog.dismiss();
            }
        });
        builder.setNegativeButton(
            "NO", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                functor.onNegativeButton(activity);
                dialog.dismiss();
            }
        });

        alert_ = builder.create();
    }

    public void show()
    {
        alert_.show();
    }
}
