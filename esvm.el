;;; esvm.el --- Emacs binding for libsvm -*- lexical-binding: t; -*-

;; Copyright (c) 2018 Abhinav Tushar

;; Author: Abhinav Tushar <lepisma@fastmail.com>
;; Version: 0.0.1
;; Package-Requires: ((emacs "25"))
;; URL: https://github.com/lepisma/esvm

;;; Commentary:

;; Emacs binding for libsvm
;; This file is not a part of GNU Emacs.

;;; License:

;; This program is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program. If not, see <http://www.gnu.org/licenses/>.

;;; Code:

(require 'esvm-core)

(defun esvm-version ()
  "Return version information"
  (format "esvm using LibSVM version %s" (esvm--libsvm-version)))

(defun esvm-fit (x-train y-train)
  "Train a classifier on the given vectors of float and return a model"
  (esvm--fit x-train y-train))

(defun esvm-predict (model x-test)
  "Return predictions from the MODEL for X-TEST"
  (esvm--predict model x-test))

(provide 'esvm)

;;; esvm.el ends here
