;; prs major-mode

(setq prs-highlights
      '(("==" . font-lock-function-name-face)
	("Whitespaces\\|Escape chars\\|Breaksymbols\\|EOF" . font-lock-constant-face)))

(define-derived-mode prs-mode text-mode "Parser"
  "Major mode for editing parser (prs) files."
  (setq font-lock-defaults '(prs-highlights)))
