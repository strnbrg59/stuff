package net.trhj.cardation;

import android.app.Activity;
import android.content.Intent;

public class RestorePopupFunctor implements PopupFunctor {

    public void onPositiveButton(Activity activity, Object extra) {
        RestoreActivity.restore_source_ = "sdcard";
        RestoreActivity.restore_attachment_ = null;
        Intent intent = new Intent(activity, RestoreActivity.class);
        activity.startActivity(intent);
    }

    public void onNegativeButton(Activity activity) {
    }
}
