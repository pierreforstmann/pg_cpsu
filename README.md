# pg_cpsu

```
shared_preload_libraries='pg_cpsu'
pg_cpsu.action_level='notice'
```


```
pierre=# select * from t;
NOTICE:  pg_cpsu: prepared statement 'select * from t;' not used (parameter number=0)
 x 
---
(0 rows)

pierre=# prepare fooplan(int) as select * from t where x=$1;
PREPARE
pierre=# execute fooplan(1);
NOTICE:  pg_cpsu: prepared statement 'prepare fooplan(int) as select * from t where x=$1;' used (parameters number=1)
 x 
---
(0 rows)
```

