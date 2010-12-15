--- apachesolr_views_query.inc	2010/12/15 10:43:34	1.37
+++ apachesolr_views_query.inc	2010/12/15 12:01:41	1.38
@@ -1,5 +1,5 @@
 <?php
-// $Id$ 
+// $Id$ 
 
 // TODO: Spellchecker field? What todo about spellchecker...
 
@@ -88,10 +88,17 @@
   protected $base_table;
   protected $sql_base_table;
   
+<<<<<<< apachesolr_views_query.inc
+  protected $base_field = 'nid';
+
+  protected $_db_query = NULL;
+  protected $has_sql_fields = FALSE;
+=======
   protected $base_field;
 
   protected $_db_query = NULL;
   protected $has_sql_fields = FALSE;
+>>>>>>> 1.37
     
   /**
    * Views init() function
@@ -101,11 +108,19 @@
     $this->base_field = $base_field;
     $this->sql_base_table = substr($base_table, 11);
     $this->_solr_service = apachesolr_get_solr();
+<<<<<<< apachesolr_views_query.inc
+
+    module_load_include('inc', 'views', 'plugins/views_plugin_query_default');
+    $this->_db_query = new views_plugin_query_default;
+    $this->_db_query->init('node', 'nid');
+
+=======
 
     module_load_include('inc', 'views', 'plugins/views_plugin_query_default');
     $this->_db_query = new views_plugin_query_default;
     $this->_db_query->init($this->sql_base_table, $base_field, $options);
 
+>>>>>>> 1.37
     $this->id = ++self::$idCount;
 
     $data = views_fetch_data($base_table);
@@ -162,7 +177,12 @@
         $solr = apachesolr_get_solr();
         $total = 0;
         if (isset($solr)) {
-          $data = $solr->getLuke();
+          try {
+            $data = $solr->getLuke();
+          }
+          catch (Exception $e) {
+            watchdog('apachesolr_views', $e->getMessage());
+          }
           if (isset($data) && isset($data->index->numDocs)) {
             $total = $data->index->numDocs;
           }
@@ -337,6 +357,38 @@
             }
           }
         }
+<<<<<<< apachesolr_views_query.inc
+
+        $nids = array();
+        foreach ($results as $doc) {
+          $nids[] = $doc->nid;
+        }
+
+        // If there has been SQL fields added via add_field(), run a SQL query.
+        if ($this->has_sql_fields) {
+          $node_table = $this->_db_query->ensure_table('node');
+          $replace = array_fill(0, sizeof($nids), '%d');
+          $in = '(' . implode(", ", $replace) . ')';
+          $this->_db_query->add_where(0, "$node_table.nid IN $in", $nids);
+          $this->_db_query->add_field($node_table, 'nid');
+
+          $sql_query = $this->_db_query->query();
+          $sql_args = $this->_db_query->get_where_args();
+          $sql_result = db_query($sql_query, $sql_args);
+
+          // Merge results from the SQL query into the $result set.
+          while ($sql_row = db_fetch_object($sql_result)) {
+            foreach ($results as $key => $doc) {
+              if ($doc->nid == $sql_row->nid) {
+                foreach ($sql_row as $field => $value) {
+                  $results[$key]->$field = $value;
+                }
+              }
+            }
+          }
+        }
+
+=======
 
         $base_fields = array();
         foreach ($results as $doc) {
@@ -367,6 +419,7 @@
           }
         }
 
+>>>>>>> 1.37
         $view->result = $results;
       }
     }
@@ -412,6 +465,7 @@
   function get_keys() {
     return $this->keys;
   }
+<<<<<<< apachesolr_views_query.inc
 
   /**
    * Add a Solr Field to retrieve.
@@ -429,6 +483,37 @@
     return $field;
   }
   
+=======
+
+>>>>>>> 1.37
+  /**
+<<<<<<< apachesolr_views_query.inc
+   * Add a field to retrieve.
+   * 
+   * $compat_field is used for compatibility with the views_plugin_query_default
+   * definition of this method - when $compat_field is set, $field is ignored
+   * and $compat_field used instead.
+   */
+  public function add_field($table, $field, $alias = '', $params = array()) {
+    $this->has_sql_fields = TRUE;
+    return $this->_db_query->add_field($table, $field, $alias, $params);
+=======
+   * Add a Solr Field to retrieve.
+   *
+   * @param $field
+   *  The name of the field to add.
+   *
+   * @return string
+   *   The field alias.
+   */
+  public function add_solr_field($field) {
+    if (!in_array($field, $this->_used_fields)) {
+      $this->_used_fields[] = $field;
+    }
+    return $field;
+>>>>>>> 1.37
+  }
+  
   /**
    * Add a field to retrieve.
    */
@@ -441,6 +526,11 @@
    * Ensure table function.
    */
   public function ensure_table($table, $relationship) {
+<<<<<<< apachesolr_views_query.inc
+    if ($table != 'apachesolr') {
+      return $this->_db_query->ensure_table($table, $relationship);
+    }
+=======
     if ($table != $this->base_table) {
       if (substr($table, 0, strlen($this->base_table)) == $this->base_table) {
         $real_table = substr($table, strlen($this->base_table) + 1);
@@ -450,6 +540,7 @@
       }
       return $this->_db_query->ensure_table($real_table, $relationship);
     }
+>>>>>>> 1.37
   }
 
   /**
