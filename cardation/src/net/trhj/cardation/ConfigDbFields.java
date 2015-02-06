package net.trhj.cardation;

public class ConfigDbFields {
        public ConfigDbFields(String lang) {
            language_ = lang;
            rand_reversal_ = ConfigActivity.default_rand_reversal_;
            batch_size_ = ConfigActivity.default_batch_size_;
            initial_streaks_ = ConfigActivity.default_streak_;
        }
        String language_;
        int rand_reversal_;
        int batch_size_;
        int initial_streaks_;
}

