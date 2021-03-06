(add-to-list 'load-path "~/.emacs.d/elisp/")
(load-library "my-config")
(load-library "view")
(load-library "styles")
(load-library "keys")
(load-library "modes")
;(load-library "compilation")

;;(shell)
;; Do NOT insert tabs into indentation
(setq-default indent-tabs-mode nil)

(setq doug-todo-file "p:/github/win32service/misc/todo.org")
(setq doug-log-file "p:/github/win32service/misc/log.org")
(setq doug-notes-file "p:/github/win32service/misc/notes.org")
(setq fill-column 110)
(setq sentence-end-double-space nil)
(setq shift-select-mode nil)
(setq enable-local-variables nil)
(setq casey-font "outline-DejaVu Sans Mono")

(setq casey-makescript "build.bat")
(setq casey-font "outline-Liberation Mono")

;; It's often nice to fill paragraphs and comments beyond the
;; default of column 70.
(setq-default fill-column 110)

(load-library "view")
(require 'ido)
(ido-mode t)

;; ====================
;; insert date and time
(defvar current-date-time-format "%a %b %d %H:%M:%S %Z %Y"
  "Format of date to insert with `insert-current-date-time' func
See help of `format-time-string' for possible replacements")

(defvar current-date-format "%Y-%m-%d"
  "Format of date to insert with `insert-current-date' function
See help of `format-time-string' for possible replacements")

(defvar current-time-format "%a %H:%M:%S"
  "Format of date to insert with `insert-current-time' func.
Note the weekly scope of the command's precision.")

(defun insert-current-date-time ()
  "insert the current date and time into current buffer.
Uses `current-date-time-format' for the formatting the date/time."
  (interactive)
  (insert (format-time-string current-date-time-format (current-time))))

(defun insert-current-time ()
  "insert the current time (1-week scope) into the current buffer."
  (interactive)
  (insert (format-time-string current-time-format (current-time))))

(defun insert-current-date()
  "insert the current date into the current buffer.
Uses `current-date-format' for formatting the date."
  (interactive)
  (insert (format-time-string current-date-format (current-time))))

(defun org-source-format ()
  "Format the given file as an org file."
  (interactive)
  (setq CurrentDate (format-time-string current-date-format (current-time)))
  (insert "#+SETUP FILE: ~/.emacs.d/org-templates/level-0.org\n")
  (insert "#+STARTUP: indent overview inlineimages\n")
  (insert "#+TITLE:\n")
  (insert (concat "#+DATE: " CurrentDate "\n"))
  (insert "#+DESCRIPTION: \n")
  (insert "#+KEYWORDS: \n")
  (insert "#+TOC: tables listings\n")
  (insert "#+CATEGORY: \n")
  (insert "#  LocalWords:  \n\n"))

(defun c-header-format ()
  "Format the given file as a header file."
  (interactive)
  (setq BaseFileName (file-name-sans-extension (file-name-nondirectory buffer-file-name)))
  (setq AuthorsName "Douglas B. Cuthbertson")
  (insert "#if !defined(")
  (push-mark)
  (insert BaseFileName)
  (upcase-region (mark) (point))
  (pop-mark)
  (insert "_H)\n\n")
  (insert "/* ========================================================================\n")
  (insert (concat "   Author: " AuthorsName "\n"))
  (insert (concat "   (C) Copyright 2015 by " AuthorsName ". All Rights Reserved.\n"))
  (insert "   ======================================================================== */\n\n")
  (insert "/**\n")
  (insert (concat " *  @file      " (file-name-nondirectory buffer-file-name) "\n"))
  (insert " *  @brief     \n")
  (insert " *  @details   \n")
  (insert " *\n")
  (insert (concat " *  @author    " AuthorsName "\n"))
  (insert " *\n")
  (insert (concat " *  @copyright 2015 by " AuthorsName ". All Rights Reserved.\n"))
  (insert " */\n")
  (insert "\n")
  (insert "#define ")
  (push-mark)
  (insert BaseFileName)
  (upcase-region (mark) (point))
  (pop-mark)
  (insert "_H\n")
  (insert "#endif\n")
)

(defun c-source-format ()
  "Format the given file as a source file."
  (interactive)
  (setq AuthorsName "Douglas B. Cuthbertson")
  (insert "/* ========================================================================\n")
  (insert (concat "   Author: " AuthorsName "\n"))
  (insert (concat "   (C) Copyright 2015 by " AuthorsName ". All Rights Reserved.\n"))
  (insert "   ======================================================================== */\n\n")
  (insert "/**\n")
  (insert (concat " *  @file      " (file-name-nondirectory buffer-file-name) "\n"))
  (insert " *  @brief     \n")
  (insert " *  @details   \n")
  (insert (concat " *  @author    " AuthorsName "\n"))
  (insert " *\n")
  (insert (concat " *  @copyright 2015 by " AuthorsName ". All Rights Reserved.\n"))
  (insert " */\n")
  (insert "\n")
)

; Setup my find-files
;;(define-key global-map "\ef" 'find-file)
;;(define-key global-map "\eF" 'find-file-other-window)

;;(global-set-key (read-kbd-macro "\eb")  'ido-switch-buffer)
;;(global-set-key (read-kbd-macro "\eB")  'ido-switch-buffer-other-window)

(defun casey-ediff-setup-windows (buffer-A buffer-B buffer-C control-buffer)
  (ediff-setup-windows-plain buffer-A buffer-B buffer-C control-buffer)
)
(setq ediff-window-setup-function 'casey-ediff-setup-windows)
(setq ediff-split-window-function 'split-window-horizontally)

; Turn off the bell
(defun nil-bell ())
(setq ring-bell-function 'nil-bell)

; Setup my compilation mode
(defun casey-big-fun-compilation-hook ()
  (make-local-variable 'truncate-lines)
  (setq truncate-lines nil)
)

(add-hook 'compilation-mode-hook 'casey-big-fun-compilation-hook)
(add-hook 'org-mode-hook
          (lambda () (visual-line-mode)))

(defun load-todo ()
  (interactive)
  (find-file doug-todo-file)
)
(define-key global-map "\et" 'load-todo)

(defun insert-timeofday ()
   (interactive "*")
   (insert (format-time-string "---------------- %a, %d %b %y: %I:%M%p")))


(defun load-doug-notes ()
  "Find from the current directory and load notes.org"
  (interactive)
  (find-file doug-notes-file)
)
(define-key global-map "\C-co" 'load-doug-notes)


(defun load-log ()
  (interactive)
  (find-file doug-log-file)
  (visual-line-mode)
  (end-of-buffer)
  (newline-and-indent)
  (insert-timeofday)
  (newline-and-indent)
  (newline-and-indent)
  (end-of-buffer)
)
(define-key global-map "\eT" 'load-log)

;; no screwing with my middle mouse button
(global-unset-key [mouse-2])

;; Bright-red TODOs
 (setq fixme-modes '(c++-mode c-mode emacs-lisp-mode))
 (make-face 'font-lock-fixme-face)
 (make-face 'font-lock-study-face)
 (make-face 'font-lock-important-face)
 (make-face 'font-lock-note-face)
 (mapc (lambda (mode)
	 (font-lock-add-keywords
	  mode
	  '(("\\<\\(TODO\\)" 1 'font-lock-fixme-face t)
        ("\\<\\(STUDY\\)" 1 'font-lock-study-face t)
	    ("\\<\\(IMPORTANT\\)" 1 'font-lock-important-face t)
            ("\\<\\(NOTE\\)" 1 'font-lock-note-face t))))
	fixme-modes)
 (modify-face 'font-lock-fixme-face "Red" nil nil t nil t nil nil)
 (modify-face 'font-lock-study-face "Yellow" nil nil t nil t nil nil)
 (modify-face 'font-lock-important-face "Yellow" nil nil t nil t nil nil)
 (modify-face 'font-lock-note-face "Dark Green" nil nil t nil t nil nil)

;; Accepted file extensions and their appropriate modes
(setq auto-mode-alist
      (append
       '(("\\.cpp$"    . c++-mode)
         ("\\.hin$"    . c++-mode)
         ("\\.cin$"    . c++-mode)
         ("\\.inl$"    . c++-mode)
         ("\\.rdc$"    . c++-mode)
         ("\\.h$"    . c++-mode)
         ("\\.c$"   . c++-mode)
         ("\\.cc$"   . c++-mode)
         ("\\.c8$"   . c++-mode)
         ("\\.txt$" . indented-text-mode)
         ("\\.emacs$" . emacs-lisp-mode)
         ("\\.gen$" . gen-mode)
         ("\\.ms$" . fundamental-mode)
         ("\\.m$" . objc-mode)
         ("\\.mm$" . objc-mode)
         ) auto-mode-alist))

(require 'cc-mode)
(require 'compile)

(setq compilation-directory-locked nil)
(setq compilation-error-regexp-alist
    (cons '("^\\([0-9]+>\\)?\\(\\(?:[a-zA-Z]:\\)?[^:(\t\n]+\\)(\\([0-9]+\\)) : \\(?:fatal error\\|warnin\\(g\\)\\) C[0-9]+:" 2 3 nil (4))
     compilation-error-regexp-alist))

;;;;;;;;;;;;;;;;;;;;;;;;;;
; C++ indentation style
(defconst casey-big-fun-c-style
  '((c-electric-pound-behavior   . nil)
    (c-tab-always-indent         . t)
    (c-comment-only-line-offset  . 0)
    (c-hanging-braces-alist      . ((class-open)
                                    (class-close)
                                    (defun-open)
                                    (defun-close)
                                    (inline-open)
                                    (inline-close)
                                    (brace-list-open)
                                    (brace-list-close)
                                    (brace-list-intro)
                                    (brace-list-entry)
                                    (block-open)
                                    (block-close)
                                    (substatement-open)
                                    (statement-case-open)
                                    (class-open)))
    (c-hanging-colons-alist      . ((inher-intro)
                                    (case-label)
                                    (label)
                                    (access-label)
                                    (access-key)
                                    (member-init-intro)))
    (c-cleanup-list              . (scope-operator
                                    list-close-comma
                                    defun-close-semi))
    (c-offsets-alist             . (
                                    (arglist-close         .  c-lineup-arglist)
;;                                    (label                 . -4)
;;                                    (access-label          . -4)
                                    (substatement-open     .  0)    ;; lines up open-brace
;;                                    (statement-case-intro  .  4)
;;                                    (statement-block-intro .  4)
;;                                    (statement-case-open   .  4)
                                    (case-label            .  4)    ;; indent case label
;;                                    (block-open            .  4)
;;                                    (inline-open           .  0)
;;                                    (topmost-intro-cont    .  0)
;;                                    (knr-argdecl-intro     . -4)
;;                                    (brace-list-open       .  0)
;;                                    (brace-list-intro      .  4)
                                    ))
    (c-echo-syntactic-information-p . t))
    "Casey's Big Fun C++ Style")


; CC++ mode handling
(defun casey-big-fun-c-hook ()
  ; Set my style for the current buffer
  (c-add-style "BigFun" casey-big-fun-c-style t)

  ; 4-space tabs
  (setq tab-width 4
        indent-tabs-mode nil)

  ; Additional style stuff
  (c-set-offset 'member-init-intro '+)
  ;;(c-set-offset 'member-init-intro '0)
  (c-set-offset 'inline-open '0)
  ; Don't indent within a C++ namespace
  (c-set-offset 'innamespace '0)
  ; Don't indent within extern "C" {} blocks
  (c-set-offset 'inextern-lang '0)

  ; No hungry backspace
  (c-toggle-auto-hungry-state -1)

  ; Newline indents, semi-colon doesn't
  (define-key c++-mode-map "\C-m" 'newline-and-indent)
  (setq c-hanging-semi&comma-criteria '((lambda () 'stop)))

  ; Handle super-tabbify (TAB completes, shift-TAB actually tabs)
  (setq dabbrev-case-replace t)
  (setq dabbrev-case-fold-search t)
  (setq dabbrev-upcase-means-case-search t)

  ; Abbrevation expansion
  (abbrev-mode 1)

  (defun casey-header-format ()
    "Format the given file as a header file."
    (c-header-format)
  )

  (defun casey-source-format ()
    "Format the given file as a source file."
    (c-source-format)
  )

  (cond ((file-exists-p buffer-file-name) t)
        ((string-match "[.]hin" buffer-file-name) (casey-source-format))
        ((string-match "[.]cin" buffer-file-name) (casey-source-format))
        ((string-match "[.]h" buffer-file-name) (casey-header-format))
        ((string-match "[.]cpp" buffer-file-name) (casey-source-format)))

  (defun casey-find-corresponding-file ()
    "Find the file that corresponds to this one."
    (interactive)
    (setq CorrespondingFileName nil)
    (setq BaseFileName (file-name-sans-extension buffer-file-name))
    (if (string-match "\\.c" buffer-file-name)
       (setq CorrespondingFileName (concat BaseFileName ".h")))
    (if (string-match "\\.h" buffer-file-name)
       (if (file-exists-p (concat BaseFileName ".c")) (setq CorrespondingFileName (concat BaseFileName ".c"))
	   (setq CorrespondingFileName (concat BaseFileName ".cpp"))))
    (if (string-match "\\.hin" buffer-file-name)
       (setq CorrespondingFileName (concat BaseFileName ".cin")))
    (if (string-match "\\.cin" buffer-file-name)
       (setq CorrespondingFileName (concat BaseFileName ".hin")))
    (if (string-match "\\.cpp" buffer-file-name)
       (setq CorrespondingFileName (concat BaseFileName ".h")))
    (if CorrespondingFileName (find-file CorrespondingFileName)
       (error "Unable to find a corresponding file")))
  (defun casey-find-corresponding-file-other-window ()
    "Find the file that corresponds to this one."
    (interactive)
    (find-file-other-window buffer-file-name)
    (casey-find-corresponding-file)
    (other-window -1))
  (define-key c++-mode-map [f12] 'casey-find-corresponding-file)
  (define-key c++-mode-map [M-f12] 'casey-find-corresponding-file-other-window)

  ; Alternate bindings for F-keyless setups (ie MacOS X terminal)
  (define-key c++-mode-map "\ec" 'casey-find-corresponding-file)
  (define-key c++-mode-map "\eC" 'casey-find-corresponding-file-other-window)

  (define-key c++-mode-map "\es" 'casey-save-buffer)

  (define-key c++-mode-map "\t" 'dabbrev-expand)
  (define-key c++-mode-map [S-tab] 'indent-for-tab-command)
;  (define-key c++-mode-map "\C-y" 'indent-for-tab-command)
  (define-key c++-mode-map [C-tab] 'indent-region)
  (define-key c++-mode-map "	" 'indent-region)

  (define-key c++-mode-map "\ej" 'imenu)

  (define-key c++-mode-map "\e." 'c-fill-paragraph)

  (define-key c++-mode-map "\e/" 'c-mark-function)

  (define-key c++-mode-map "\e " 'set-mark-command)
  (define-key c++-mode-map "\eq" 'append-as-kill)
;  (define-key c++-mode-map "\ea" 'yank)
  (define-key c++-mode-map "\ez" 'kill-region)

  ; devenv.com error parsing
  (add-to-list 'compilation-error-regexp-alist 'casey-devenv)
  (add-to-list 'compilation-error-regexp-alist-alist '(casey-devenv
   "*\\([0-9]+>\\)?\\(\\(?:[a-zA-Z]:\\)?[^:(\t\n]+\\)(\\([0-9]+\\)) : \\(?:see declaration\\|\\(?:warnin\\(g\\)\\|[a-z ]+\\) C[0-9]+:\\)"
    2 3 nil (4)))

  ; Turn on line numbers
  ;(linum-mode)
)


(defun find-project-directory-recursive ()
  "Recursively search for a makefile."
  (interactive)
  (if (file-exists-p (concat "misc/" casey-makescript)) (cd "misc")
      (cd "../")
      (find-project-directory-recursive)))

(defun lock-compilation-directory ()
  "The compilation process should NOT hunt for a makefile"
  (interactive)
  (setq compilation-directory-locked t)
  (message "Compilation directory is locked."))

(defun unlock-compilation-directory ()
  "The compilation process SHOULD hunt for a makefile"
  (interactive)
  (setq compilation-directory-locked nil)
  (message "Compilation directory is roaming."))

(defun find-project-directory ()
  "Find the project directory."
  (interactive)
  (progn
    (message "Find the project directory\n")
    (message "The default directory is %s\n" default-directory)
    (setq find-project-from-directory default-directory)
    (switch-to-buffer-other-window "*compilation*")
    (if compilation-directory-locked (cd last-compilation-directory)
      (cd find-project-from-directory)
      (find-project-directory-recursive)
      (setq last-compilation-directory default-directory))))

(defun make-without-asking ()
  "Make the current build."
  (interactive)
  (if (find-project-directory) (compile casey-makescript))
  (other-window 1))
(define-key global-map "\em" 'make-without-asking)
;;;;;;;;;;;;;;;;;;;;;;;;;;

(defun casey-replace-string (FromString ToString)
  "Replace a string without moving point."
  (interactive "sReplace: \nsReplace: %s  With: ")
  (save-excursion
    (replace-string FromString ToString)
  ))
(define-key global-map [f8] 'casey-replace-string)

(add-hook 'c-mode-common-hook 'casey-big-fun-c-hook)

(defun casey-save-buffer ()
  "Save the buffer after untabifying it."
  (interactive)
  (save-excursion
    (save-restriction
      (widen)
      (untabify (point-min) (point-max))))
  (save-buffer))

;; TXT mode handling
(defun casey-big-fun-text-hook ()
  ; 4-space tabs
  (setq tab-width 4
        indent-tabs-mode nil)

  ; Newline indents, semi-colon doesn't
  (define-key text-mode-map "\C-m" 'newline-and-indent)

  ; Prevent overriding of alt-s
  (define-key text-mode-map "\es" 'casey-save-buffer)
  )
(add-hook 'text-mode-hook 'casey-big-fun-text-hook)

;; Window Commands
(defun w32-restore-frame ()
    "Restore a minimized frame"
     (interactive)
     (w32-send-sys-command 61728))

(defun maximize-frame ()
    "Maximize the current frame"
     (interactive)
     (w32-send-sys-command 61488))

(define-key global-map "\ep" 'maximize-frame)
;;(define-key global-map "\ew" 'other-window)

;; Navigation
(defun previous-blank-line ()
  "Moves to the previous line containing nothing but whitespace."
  (interactive)
  (search-backward-regexp "^[ \t]*\n")
)

(defun next-blank-line ()
  "Moves to the next line containing nothing but whitespace."
  (interactive)
  (forward-line)
  (search-forward-regexp "^[ \t]*\n")
  (forward-line -1)
)

(define-key global-map [C-right] 'forward-word)
(define-key global-map [C-left] 'backward-word)
(define-key global-map [C-up] 'previous-blank-line)
(define-key global-map [C-down] 'next-blank-line)
(define-key global-map [home] 'beginning-of-line)
(define-key global-map [end] 'end-of-line)
(define-key global-map [pgup] 'forward-page)
(define-key global-map [pgdown] 'backward-page)
(define-key global-map [C-next] 'scroll-other-window)
(define-key global-map [C-prior] 'scroll-other-window-down)

(defun append-as-kill ()
  "Performs copy-region-as-kill as an append."
  (interactive)
  (append-next-kill)
  (copy-region-as-kill (mark) (point))
)
;;(define-key global-map "\e " 'set-mark-command)
(define-key global-map "\e " 'just-one-space)
(define-key global-map "\eq" 'append-as-kill)
;;(define-key global-map "\ea" 'yank)
(define-key global-map "\ez" 'kill-region)
(define-key global-map [M-up] 'previous-blank-line)
(define-key global-map [M-down] 'next-blank-line)
(define-key global-map [M-right] 'forward-word)
(define-key global-map [M-left] 'backward-word)

;;(define-key global-map "\e:" 'View-back-to-mark)
;;(define-key global-map "\e;" 'exchange-point-and-mark)

(define-key global-map [f9] 'first-error)
(define-key global-map [f10] 'previous-error)
(define-key global-map [f11] 'next-error)

(define-key global-map "\en" 'next-error)
(define-key global-map "\eN" 'previous-error)

(define-key global-map "\eg" 'goto-line)
(define-key global-map "\ej" 'imenu)

;; Editing
;;(define-key global-map "\C-q" 'copy-region-as-kill)
;;(define-key global-map "\C-v" 'yank)
;;(define-key global-map "\C-y" 'nil)
;;(define-key global-map "\C-e" 'rotate-yank-pointer)
;;(define-key global-map "\eu" 'undo)
(define-key global-map "\e6" 'upcase-word)
(define-key global-map "\e^" 'capitalize-word)
(define-key global-map "\e." 'fill-paragraph)

(defun casey-replace-in-region (old-word new-word)
  "Perform a replace-string in the current region."
  (interactive "sReplace: \nsReplace: %s  With: ")
  (save-excursion (save-restriction
		    (narrow-to-region (mark) (point))
		    (beginning-of-buffer)
		    (replace-string old-word new-word)
		    ))
  )
(define-key global-map "\el" 'casey-replace-in-region)

(define-key global-map "\eo" 'query-replace)
(define-key global-map "\eO" 'casey-replace-string)

;; \377 is alt-backspace
(define-key global-map "\377" 'backward-kill-word)
(define-key global-map [M-delete] 'kill-word)

(define-key global-map "\e[" 'start-kbd-macro)
(define-key global-map "\e]" 'end-kbd-macro)
(define-key global-map "\e'" 'call-last-kbd-macro)

;; Buffers
(define-key global-map "\er" 'revert-buffer)
(define-key global-map "\ek" 'kill-this-buffer)
(define-key global-map "\es" 'save-buffer)

;; Commands
;; (set-variable 'grep-command "grep -irHn ")
(setq grep-use-null-device t)
(set-variable 'grep-command "findstr -s -n -i -l ")

;; prevent issues with the Windows null device (NUL)
;; when using cygwin find with rgrep.
(defadvice grep-compute-defaults (around grep-compute-defaults-advice-null-device)
  "Use cygwin's /dev/null as the null-device."
  (let ((null-device "/dev/null"))
	ad-do-it))
(ad-activate 'grep-compute-defaults)


;; Smooth scroll
;; scroll one line at a time (less "jumpy" than defaults)
(setq mouse-wheel-scroll-amount '(1 ((shift) . 1))) ;; one line at a time
(setq mouse-wheel-progressive-speed nil) ;; don't accelerate scrolling
(setq mouse-wheel-follow-mouse 't) ;; scroll window under mouse
(setq scroll-step 1) ;; keyboard scroll one line at a time
(setq auto-window-vscroll nil)  ; prevent window jump to center while scrolling
(setq scroll-conservatively 1000) ; prevent auto centering


;; Clock
(display-time)

;; Startup windowing
(setq next-line-add-newlines nil)
(setq-default truncate-lines t)
(setq truncate-partial-width-windows nil)
;;(split-window-horizontally)

(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(auto-save-default nil)
 '(auto-save-interval 0)
 '(auto-save-list-file-prefix nil)
 '(auto-save-timeout 0)
 '(auto-show-mode t t)
 '(calendar-date-style (quote iso))
 '(delete-auto-save-files nil)
 '(delete-old-versions (quote other))
 '(exec-path
   (quote
    ("c:/WINDOWS/system32" "C:/WINDOWS" "C:/WINDOWS/System32/Wbem" "C:/WINDOWS/System32/WindowsPowerShell/v1.0/" "C:/ProgramData/Oracle/Java/javapath" "C:/Program Files (x86)/NVIDIA Corporation/PhysX/Common" "C:/Apps/nodejs/" "C:/WINDOWS/system32/config/systemprofile/.dnx/bin" "C:/Program Files/Microsoft DNX/Dnvm/" "C:/Program Files (x86)/Windows Kits/10/Windows Performance Toolkit/" "C:/Program Files/Microsoft/Web Platform Installer/" "C:/Program Files (x86)/Microsoft SDKs/Azure/CLI/wbin" "C:/Program Files (x86)/QuickTime/QTSystem/" "C:/Program Files (x86)/Calibre2/" "C:/Apps/Anaconda2" "C:/Apps/Anaconda2/Scripts" "C:/Apps/Anaconda2/Library/bin" "E:/Apps/Ruby22/bin" "C:/Program Files/Common Files/Microsoft Shared/Windows Live" "C:/Program Files (x86)/Common Files/Microsoft Shared/Windows Live" "C:/Users/Doug/Apps/bin" "C:/Users/Doug/Apps/emacs/bin" "C:/Users/Doug/Apps/Aspell/bin" "C:/Apps/Tcl/bin" "C:/Apps/CMake/bin" "C:/Program Files (x86)/WinMerge" "C:/Program Files/PostgreSQL/9.2/bin" "C:/Users/Doug/Apps/julia" "C:/Apps/TexLive/2013/bin/win32" "C:/Users/Doug/Apps/Sysinternals" "C:/Program Files/Debugging Tools for Windows (x64)" "C:/Program Files/smartmontools/bin" "C:/Apps/Pandoc" "C:/Program Files/TortoiseSVN/bin" "C:/Program Files/doxygen/bin" "C:/Program Files (x86)/Graphviz2.38/bin" "C:/Program Files (x86)/Google/google_appengine/" "C:/Apps/Python27/" "C:/Apps/Python27/Scripts/" "C:/Apps/Python27/DLLs" "C:/Apps/Python27/LIB" "C:/Program Files/Boot2Docker for Windows" "C:/Program Files/PostgreSQL/9.4/bin" "C:/Users/Doug/AppData/Roaming/npm" "C:/Users/Doug/AppData/Local/Programs/Git/cmd" "C:/Users/Doug/AppData/Local/atom/bin" "c:/Users/Doug/Apps/emacs/libexec/emacs/24.5/i686-pc-mingw32" "c:/Users/Doug/Apps/cygwin64/bin")))
 '(imenu-auto-rescan t)
 '(imenu-auto-rescan-maxout 500000)
 '(initial-frame-alist (quote ((fullscreen . maximized))))
 '(kept-new-versions 5)
 '(kept-old-versions 5)
 '(make-backup-file-name-function (quote ignore))
 '(make-backup-files nil)
;; '(mouse-wheel-follow-mouse nil)
;; '(mouse-wheel-progressive-speed nil)
;; '(mouse-wheel-scroll-amount (quote (15)))
 '(version-control nil)
 '(initial-frame-alist (quote ((fullscreen . maximized))))
 '(word-wrap t)
 )

(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(org-level-1 ((t (:inherit outline-1 :foreground "cornflower blue"))))
 '(org-level-2 ((t (:inherit outline-2 :foreground "SeaGreen1"))))
 '(org-level-3 ((t (:inherit outline-3 :foreground "sienna1"))))
 '(org-level-4 ((t (:inherit outline-4 :foreground "light slate blue"))))
 '(org-level-5 ((t (:inherit outline-5 :foreground "lawn green"))))
 '(org-level-6 ((t (:inherit outline-6 :foreground "LightGoldenrod3"))))
 '(org-level-7 ((t (:inherit outline-7 :foreground "RoyalBlue1")))))


(define-key global-map "\t" 'dabbrev-expand)
(define-key global-map [S-tab] 'indent-for-tab-command)
(define-key global-map [backtab] 'indent-for-tab-command)
;;(define-key global-map "\C-y" 'indent-for-tab-command)
(define-key global-map [C-tab] 'indent-region)
(define-key global-map [C-M-\\] 'indent-region)

(define-key global-map "\M-\ " 'just-one-space)

;; (defun casey-never-split-a-window
;;     "Never, ever split a window.  Why would anyone EVER want you to do that??"
;;     nil)
;; (setq split-window-preferred-function 'casey-never-split-a-window)

(add-to-list 'default-frame-alist '(font . "Liberation Mono-11.5"))
(set-face-attribute 'default t :font "Liberation Mono-11.5")
(set-face-attribute 'font-lock-builtin-face nil :foreground "#DAB98F")
(set-face-attribute 'font-lock-comment-face nil :foreground "gray50")
(set-face-attribute 'font-lock-constant-face nil :foreground "olive drab")
(set-face-attribute 'font-lock-doc-face nil :foreground "gray50")
(set-face-attribute 'font-lock-function-name-face nil :foreground "burlywood3")
(set-face-attribute 'font-lock-keyword-face nil :foreground "DarkGoldenrod3")
(set-face-attribute 'font-lock-string-face nil :foreground "olive drab")
(set-face-attribute 'font-lock-type-face nil :foreground "burlywood3")
(set-face-attribute 'font-lock-variable-name-face nil :foreground "burlywood3")

(defun post-load-stuff ()
  (interactive)
  (menu-bar-mode -1)
  (maximize-frame)
  (set-foreground-color "burlywood3")
  (set-background-color "#161616")
  (set-cursor-color "#40FF40")
)
(add-hook 'window-setup-hook 'post-load-stuff t)
