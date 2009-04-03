<?php

/* $Id$ */

/**
 * Class that allows searching the site with Apache Solr through a view.
 */
class apachesolr_views_handler_filter_search extends views_handler_filter_search {
  
  public function query() {
    if (!empty($this->value)) {
      $this->query->set_query($this->value);
    }
  }
  
}
