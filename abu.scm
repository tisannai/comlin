(list
 (cons "comlin"
       (list (cons 'project-type          "c-library-shared")
             (cons 'source-directory      "src")
             (cons 'ceedling-options      (list "-Wall" "-fPIC" "-g" "-O0"))
             (cons 'version               "0.0.1")
             (cons 'actions               (list "version" "clean" "compile" "link" "publish"))))
 )
