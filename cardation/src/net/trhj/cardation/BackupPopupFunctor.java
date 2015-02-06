package net.trhj.cardation;

import android.app.Activity;

public class BackupPopupFunctor implements PopupFunctor {
    public void onPositiveButton(Activity activity, Object extra) {
        CardDb.backup(activity);
    }

    public void onNegativeButton(Activity activity) {
    }
}
