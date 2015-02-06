package net.trhj.cardation;

import android.app.Activity;

public class RestorePopupFunctor implements PopupFunctor {
    public void onPositiveButton(Activity activity, Object extra) {
        CardDb.restore(activity);
    }

    public void onNegativeButton(Activity activity) {
    }
}
