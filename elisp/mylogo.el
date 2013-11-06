;;
;; A (shell-command "logo testme.lg") makes logo draw its picture but
;; immediately exit.
;;
;; If I launch "logo testme.lg" from (compile), the
;; logo window stays up but logo itself is halted, so moving the window
;; makes the drawing disappear, and in any case you don't get anywhere
;; typing logo commands into the *compile* buffer.
;;
;; The solution was to do pass "xterm -e logo...&" to shell-command.
;; That creates an *Async Shell Command* buffer from which we can
;; programmatically kill logo.  Or manually change to that buffer and kill logo
;; with a "\C-c \C-c".
;;

(defun relogo ()
    "Kill any existing logo process, run logo on current buffer."
    (interactive)
    (defun yes-or-no-p (prompt) t) ; Suppresses pesky y|n prompt
    (save-buffer)
    (and (bufferp "*Async Shell Command*")
         (delete-process (get-buffer "*Async Shell Command*")))
    (shell-command (concat "xterm -e \"logo " (buffer-file-name) "\"&")))

;; Note on getting rid of the "A command is running.  Kill it? (yes or no)"
;; prompt.  It comes from the yes-or-no-p function, and after several failed
;; approaches, I hit upon redefining that function, inside relogo().
;; One failed approach: substituting this...
;;
;;       (save-excursion
;;           (progn (set-buffer (get-buffer "*Async Shell Command*"))
;;                  (comint-interrupt-subjob))))
;;
;; ...in place of the delete-process (above): didn't work.
;; Ditto removing the "(interactive)"; prompt still there.
