(defun my-save-and-compile ()
  (interactive "")
  (save-buffer 0)
  (compile "make -C ~/code/build -k"))
