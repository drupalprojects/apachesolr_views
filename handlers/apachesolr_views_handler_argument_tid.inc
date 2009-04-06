<?php
// $Id$

class apachesolr_views_handler_argument_tid extends views_handler_argument_numeric {
  
  function title_query() {
    $term_names = array();
    
    $placeholders = implode(', ', array_fill(0, sizeof($this->value), '%d'));
    
    $result = db_query("SELECT td.name FROM {term_data} td WHERE td.vid = %d AND td.tid IN ($placeholders)", $this->definition['vid'], $this->value);
    
    while ($term = db_fetch_object($result)) {
      $term_names[] = check_plain($term->name);
    }
    return $term_names;
  }
  
  /**
   * Add argument to query.
   */
  public function query() {
    // TODO: handle multiple tids
    $this->query->add_filter($this->real_field, apachesolr_views_query::escape_term($this->argument));
  }
}