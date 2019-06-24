;; bnf major-mode

(setq bnf-highlights
      '((";;.*?include" . font-lock-preprocessor-face)
	(";;.*?include.*?\\((.*?)\\)" . (1 font-lock-string-face))
	(".*?\\(;.*\\)" . (1 font-lock-comment-face))
	("\\(<.+?>\\).*::=" . (1 font-lock-function-name-face))
	("\\(<.+?>\\)" . (1 font-lock-constant-face))
	("\\(\".*?\"\\)" . (1 font-lock-string-face))))

(define-derived-mode bnf-mode text-mode "BNF"
  "Major mode for editing Bachus-Naur Form (BNF) syntax declaration files."
  (setq font-lock-defaults '(bnf-highlights)))
