;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Use cleanup-template-messages to trim template-related stuff in C++
;; compiler messages down to something that's easier to read.
;;
;; Uses *scratch* buffer for temporary storage.
;;
;; Author: Ted Sternberg
;; Date:   June 29, 2007
;;
;; Caveats:
;;  1. Leaves your kill ring all messed up.

(defun cleanup-template-messages ()
    "Generally clean up template-laced g++ error messages by removing stuff\n
     most people aren't interested in."
    (interactive)

    ;; Delete contents of *scratch* buffer, before proceeding to save the
    ;; whole current buffer there.
    (save-excursion
        (set-buffer (get-buffer "*scratch*"))
        (erase-buffer))

    (copy-to-buffer (get-buffer "*scratch*") (point-min) (point-max))
    (save-excursion
        (progn
            (replace-string "std::map" "map" () (point-min) (point-max))
            (replace-string "std::string" "string" () (point-min) (point-max))
            (replace-string "std::basic_string" "stringXXX" () (point-min) (point-max))
            ;(basenames) ;; Nice, but now "\C-x~" can't find next error.
            (delete-template "XXX<")
            (delete-template "std::less<")
            (delete-template "std::_Rb_tree_iterator<")
            (delete-template "std::allocator<")
            (replace-regexp "\\(,[ \n]*\\)+>" ">" nil (point-min) (point-max)))))

(defun delete-template (str)
    "Remove text from beginning of str< and through closing '>', repeatedly
     until there are no more matches.
     Not a public interface -- used by cleanup-template-messages."
    (goto-char (point-min))
    (while (search-forward str () t)
       (progn 
          (goto-char (- (point) (length str)))
          (push-mark)

          ; Now find the closing angle bracket.
          (goto-char (+ (point) (length str) 1))
          (let ((n_open 1) (n_close 0))
              (while (not (= n_open n_close))
                  (if (search-forward-regexp "[\<\>]" () t)
                      (progn
                        (if (string= (match-string 0) "<")
                          (setq n_open (+ n_open 1))
                          (setq n_close (+ n_close 1)))
                        (kill-region (mark) (point)))
                      (setq n_open n_close)))))))

(defun basenames ()
    "Applies something like /bin/basename to everything that looks like
    a path"
    (while
      (search-forward-regexp "/\\([^ \n]+/\\)+\\([^ \n]+\\)+" (point-max) t)
      (replace-match "\\2" t nil nil nil)))


(global-set-key "\C-cr" 'search-forward-regexp)

(defun restore-template-messages ()
    "Bring back the long-style messy messages that cleanup-template-messages
    simplified for us."
    (interactive)
    (kill-region (point-min) (point-max))
    (goto-char (point-min))
    (insert-buffer (get-buffer "*scratch*")))
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
