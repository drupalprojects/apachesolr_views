<?php

// $Id$ 

// TODO: Spellchecker field? What todo about spellchecker...

/**
 * Class for handling a view that gets its data not from the database, but from
 * a Solr server.
 */
class apachesolr_views_query extends views_plugin_query implements Drupal_Solr_Query_Interface {
  
  /**
   * Static shared by all instances, used to increment ID numbers.
   */
  protected static $idCount = 0;

  /**
   * Each query/subquery will have a unique ID
   */
  public $id;
  
  // facets for the solr query
  protected $_facets = array();
  
  // String containing the query that will be handed to Solr.
  protected $_query;
  
  // Object encapsulating the actual query to Solr.
  protected $_solr_service;
  
  // Array containing the parameters that will be handed to Solr.
  protected $_params = array();
  
  // array all of the fields for the query
  protected $_used_fields = array(
    'id',
    'nid',
    'url',
    'uid', // TODO: skip this probably, but not for user searching...
  );
  
  // the template to use on the solr
  protected $_query_template = '';
  
  // the view object. used for the get_path() method
  // @see get_path()
  protected $_view_arguments = array();
  
  // from the view object used in the get_url_querystring() method
  // @see get_url_querystring()
  protected $_view_filters = array();
  
  // this stores the base path for the view. we store it here as the
  // $view object isn't always avaible
  protected $_base_path = '';
  
  // used to store subqueries. Useful in say the node_access implementation
  protected $_subqueries = array();
  
  // array to store extra url query params. Used for instance to store
  // latitudes and longitudes
  protected $_extra_query = array();
  
  // array to store the sorts for get_available_sorts() call
  protected $_available_sorts = array();
  
  // string to identify the current Views display
  protected $_current_display = '';
  
  // string to identify the current Views name
  protected $_current_views_name = '';
  
  /**
   * Views init() function
   */
  public function init($base_table, $base_field) {
    $this->_solr_service = apachesolr_get_solr();
    $this->id = ++self::$idCount;
  }
  
  /**
   * Constructor build up the avialable sorts
   */
  public function __construct() {
    $data = _views_fetch_data('apachesolr');
    foreach ($data as $field_name => $field) {
      if (!empty($field['sort'])) {
        $this->_available_sorts[$field_name] = array(
          'name' => $field['title'],
          'default' => 'asc', // TODO: make this part of the handler def? So it will be in the Views data defintion
        );
      }
    }
  }
  
  /**
   * copy constructor... what else to do?
   */
  public function __clone() {
    $this->id = ++self::$idCount;
  }
  
  /**
   * Build the query object. Load up all avaivable facets so the blocks work.
   */
  public function build() {    
    try {
      // TODO: add in config screen on teh View instead of using apachesolr settings
      // NOTICE: we override the fl further down in this process. And the rows param
      // does not take affect here as we set the rows in the execute() method
      $this->_params += apachesolr_search_basic_params($this);
      apachesolr_search_add_facet_params($this->_params, $this);
      apachesolr_search_add_boost_params($this->_params, $this, $this->_solr_service);
      
      // Add in facets now
      $this->_params['fq'] = $this->rebuild_fq();
      
      
      // process subqueries
      foreach ($this->_subqueries as $query_data) {
        // process the query string first
        $sub_query_string = $query_data['query']->get_query_basic();
        if ($sub_query_string) {
          $this->_query .= " {$query_data['q_op']} ({$sub_query_string})";
        }
        
        // now handle the filter query
        $subfq = $query_data['query']->get_fq();
        if (!empty($subfq)) {
          $operator = $query_data['fq_op'];
          $this->_params['fq'][] = "(" . implode(" {$operator} ", $subfq) .")";
        }
        
      }
      
      // add in the fields. These fields include the necessary fields.
      $this->_params['fl'] = implode(',', $this->_used_fields);
      
      // query template handling
      if (!empty($this->_query_template)) {
        $this->_params['qt'] = $this->_query_template;
      }
    }
    catch (Exception $e) {
      watchdog('apachesolr_views', $e->getMessage());
    }
  }
  
  /**
   * Let modules modify the query just prior to finalizing it.
   */
  public function alter(&$view) {
    foreach (module_implements('apachesolr_modify_query') as $module) {
      $function = $module . '_apachesolr_modify_query';
      $function($this, $this->_params, 'apachesolr_views_query');
    }
  }
  
  /**
   * Executes the query and fills the associated view object with according
   * values.
   * 
   * Values to set: $view->result, $view->total_rows, $view->execute_time,
   * $view->pager['current_page'].
   */
  public function execute(&$view) {
    $start_time = views_microtime();
    $view->result = array();
    $this->_view_arguments = $view->argument;
    $this->_view_filters = $view->filter;
    $this->_base_path = $view->display[$view->current_display]->display_options['path'];
    $this->_current_display = $view->current_display;
    $this->_current_views_name = $view->name;

    try {
      // TODO: handle multiple pages seperated by commas. @see views_plugin_query_default.inc
      $page = isset($_GET['page']) ? $_GET['page'] : 0;
      $offset = $page * $view->pager['items_per_page'];
      
      $response = $this->_solr_service->search($this->_query, $offset, $view->pager['items_per_page'], $this->_params);
      
      $view->total_rows = $total = $response->response->numFound;
      
      // we do a fake pager here. Set the globals so the pager works (tm)
      if (!empty($view->pager['use_pager'])) {
        $view->pager['current_page'] = $offset / $view->pager['items_per_page'];
        
        // OOO! globals. This is to make paging work..
        global $pager_page_array, $pager_total, $pager_total_items;
        
        // Set the item count for the pager.
        $pager_total_items[$view->pager['element']] = $view->total_rows;
        
        // Calculate and set the count of available pages.
        $pager_total[$view->pager['element']] = ceil($pager_total_items[$view->pager['element']] / $view->pager['items_per_page']);

        // What page was requested:
        // TODO: handle multiple page requests seperated by commas.
        $pager_page_array = array($page);
        $pager_page_array[$view->pager['element']] = $view->pager['current_page'];
      }
      
      // The response is cached so that it is accessible to the blocks and
      // anything else that needs it beyond the initial search.
      apachesolr_static_response_cache($response);
      apachesolr_has_searched(TRUE);
      apachesolr_current_query($this);
      
      if ($total > 0) {
        $results = $response->response->docs;
        
        // Process dates
        $date_fields = array('created', 'changed');
        foreach (array_values($date_fields) as $field) {
          if (empty($this->_used_fields[$field])) {
            unset($date_fields[$field]);
          }
        }
        if (!empty($date_fields)) {
          foreach ($results as $doc) {
            foreach ($date_fields as $field) {
              $doc->$field = strtotime($doc->$field);
            }
          }
        }
        
        $view->result = $results;
      }
    }
    catch (Exception $e) {
      watchdog('Apache Solr', $e->getMessage(), NULL, WATCHDOG_ERROR);
      apachesolr_failure(t('Solr search'), is_null($query) ? $this->_keys : $query->get_query_basic());
    }
    
    $view->execute_time = views_microtime() - $start_time;
  }
  
  
  /**
   * Add a field to retrieve.
   * 
   * $compat_field is used for compatibility with the views_plugin_query_default
   * definition of this method - when $compat_field is set, $field is ignored
   * and $compat_field used instead.
   */
  public function add_field($field, $compat_field = NULL) {
    if (!empty($compat_field)) {
      $field = $compat_field;
    }
    if (is_array($field) && isset($field['field'])) {
      $field = $field['field'];
    }
    if (empty($field) || !is_string($field)) {
      return FALSE;
    }
    if (!in_array($field, $this->_used_fields)) {
      $this->_used_fields[] = $field;
    }
    return $field;
  }
  
  /**
   * Add a sorting directive.
   * 
   * @param $single If TRUE, the results will only be sorted by this order.
   */
  public function add_sort($field, $order, $single = FALSE) {
    $sort = $field . ' ' . strtolower($order);
    if (empty($this->_params['sort']) || $single) {
      $this->_params['sort'] = $sort;
    }
    else {
      $this->_params['sort'] .= ',' . $sort;
    }
  }
  
  /**
   * get a search param. Used primarly by the snippet field
   *
   * @param string $param_name
   *
   * @return string parameter setting
   */
  function get_param($param_name) {
    return $this->_params[$param_name];
  }
  
  /**
   * set the search param. useful for extensions like localsolr
   *
   * @param string $param_name
   * name of the parameter
   *
   * @param string $param_value
   * value of the parameter
   *
   * @return none;
   */
  function set_param($param_name, $param_value) {
    $this->_params[$param_name] = $param_value;
  }
  
  /**
   * Add in a facet string
   *
   * @param string $type
   * The type of facet. use apachesolr_index_key() for dynamic fields
   *
   * @param string $value
   * The value of the facet. Can be "story OR page" to filter multiple
   *
   * @param boolean $exclude
   * Whether or not to exclude the value from the results
   *
   * @return none
   */
  public function add_filter($type, $value, $exclude = FALSE) {
    $this->_facets[$type][] = array(
      'value' => $value,
      'exclude' => $exclude,
    );
  }
  
  /**
   * This function sets the query string
   *
   * @param string $query
   * plain text typed in search query
   *
   * @return none
   */
  public function set_query($query) {
    return $this->_query = $query;
  }
  
  /**
   * return the basic query
   */
  public function get_query_basic() {
    return $this->_query;
  }
  
  /**
   * Checks to see if the facet has bene applied
   *
   * @param string $field
   * the facet field to check
   *
   * @param string $value
   * The facet value to check against
   */
  function has_filter($field, $value) {
    if (isset($this->_facets[$field])) {
      foreach ($this->_facets[$field] as $definition) {
        if ($definition['value'] == $value) {
          return TRUE;
        }
      }
    }
    
    return FALSE;
  }
  
  /**
   * Remove a facet from the query
   *
   * @param string $field
   * the facet field to remove
   *
   * @param string $value
   * The facet value to remove
   * This value can be NULL
   */
  function remove_filter($field, $value = NULL) {
    if (!empty($value)) {
      if (isset($this->_facets[$field])) {
        $removal_key = FALSE;
        foreach ($this->_facets[$field] as $key => $definition) {
          if ($definition['value'] == $value) {
            $removal_key = $key;
            break;
          }
        }
        // we found it delete the value
        if ($removal_key !== FALSE) {
          unset($this->_facets[$field][$removal_key]);
        }
      }
    }
    else if (isset($this->_facets[$field])) {
      unset($this->_facets[$field]);
    }
  }
  
  /**
   * return the search path
   */
  function get_path() {
    $wildcard_count = 0; // this is used to remove the some/path/argument/all/all paths
    if (empty($this->_view_arguments)) {
      return $_GET['q'];
    }
    foreach ($this->_view_arguments as $field => $argument) {
      // because some arguments arn't standard base arguments we need to do things differently
      // so that the facet blocks behave. So any argument thats not part of a facet block
      // we have its value copied into the path
      // arguments that are a facet block argument will be processed differently
      $path_part = $this->part_of_facet_block($argument) ? $this->argument_part($field) : $argument->argument;
      if (!empty($path_part)) {
        $path_parts[$argument->position + 1] = $path_part;
        $wildcard_count = 0;
      }
      else {
        $path_parts[$argument->position + 1] = $argument->options['wildcard'];
        $wildcard_count++;
      }
    }
    $arguments = explode('/', $this->_base_path);
    $path = '';
    foreach ($arguments as $arg) {
      if ($arg == '%') {
        $part = array_shift($path_parts);
      }
      else {
        $part = $arg;
      }
      $path .= "/$part";
    }
    
    if (count($path_parts)) {
      $path .= "/" .  implode('/', array_slice($path_parts, 0, count($path_parts) - $wildcard_count));
    }
    return substr($path, 1);
  }
  
  /**
   * return the facet argument suitable for Views
   *
   * @param string $field
   * name of the facet
   *
   * @return string the part of the url for Views that matches this facet value
   */
  protected function argument_part($field) {
    $argument_path = '';
    if (empty($this->_facets[$field])) {
      return '';
    }
    foreach ($this->_facets[$field] as $defintion) {
      if (!$defintion['exclude']) {
        $argument_path .= ',' . $defintion['value'];
      }
    }
    
    return substr($argument_path, 1);
  }
  
  /**
   * determine if argument field is part of a facet block
   *
   * @return bool
   * Whether or not the provided argument could be in a facet block
   */
  protected function part_of_facet_block($argument) {
    $facet_arguments = array(
      'tid',
      'type',
      'uid',
    );
    // we use field instead of real_field
    // because we want these specific ones defined in
    // apachesolr_views.views.inc
    return (in_array($argument->field, $facet_arguments));
  }
  
  /**
   * return any query string for use in the l function
   *
   * @see l()
   */
  function get_url_querystring() {
    // goes through and finds all the exposed filters
    $query_string = '';
    foreach ($this->_view_filters as $filter) {
      if ($filter->options['exposed']) {
        if ($filter->field == 'text' && !empty($this->_query)) {
          // the search exposed filter is special
          $query_string .= '&' . $filter->options['expose']['identifier'] . '=' . $this->_query;
        }
        else if ($this->_facets[$filter->field]) {
          $query_string .= '&' . $filter->options['expose']['identifier'] . '=' . $this->_facets[$filter->field];
        }
        else {
          $query_string .= '&' . $filter->options['expose']['identifier'] . '=' . $_GET[$filter->options['expose']['identifier']];
        }
      }
    }
    
    foreach ($this->_extra_query as $field => $value) {
      $query_string .= "&$field=$value";
    }
    
    return !empty($query_string) ? substr($query_string, 1) : NULL;
  }
  
  /**
   * set the solrsort. This currently doesn't work
   *
   * @param string raw string to set the sort to
   */
  function set_solrsort($sortstring) {
    list($type, $direction) = explode(" ", $sortstring);
    $this->add_sort($type, $direction);
  }
  
  /**
   * return an array of filters
   *
   * @param string $name
   * the name of the filter applied to the query
   * 
   * Compabatiablity layer with Solr_Base_Query
   */
  function get_filters( $name = NULL) {
    $filters = array();
    foreach ($this->_facets as $type => $fields) {
      foreach ($fields as $data) {
        $filters[] = array(
          '#name' => ($data['exclude']) ? "NOT $type" : $type, 
          '#value' => $data['value']
        );
      }
    }
    
    // if we are looking for a specific one
    // remove all those that don't match and return the array
    if (!empty($name)) {
      foreach ($filters as $key => $filter) {
        if ($filter['#name'] != $name) {
          unset($filters[$key]);
        }
      }
    }
    
    return $filters;
  }
  
  function get_fq() {
    return $this->rebuild_fq();
  }
  
  /**
   * return the Views display id
   */
  function get_display_id() {
    return $this->_current_display;
  }
  
  /**
   * return the Views name
   */
  function get_views_name() {
    return $this->_current_views_name;
  }
  
  /**
   * add a subquery to the current query
   */
  function add_subquery(Drupal_Solr_Query_Interface $query, $fq_operator = 'OR', $q_operator = 'AND') {
    $this->_subqueries[$query->id] = array(
      'query' => $query,
      'fq_op' => $fq_operator,
      'q_op' => $q_operator,
    );
  }
  /**
   * Escapes a term for passing it to the query.
   */
  public static function escape_term($term) {
    $term = trim($term);
    if (empty($term)) {
      return '';
    }
    if (($term{0} == '"' && $term{strlen($term)-1} == '"')
        || $term{0} == '(' && $term{strlen($term)-1} == ')') {
      return $term;
    }
    
    if (strpos($term, ' ') !== FALSE) {
      return Drupal_Apache_Solr_Service::phrase($term);
    }
    return Drupal_Apache_Solr_Service::escape($term);
  }
  
  /**
   * allows the user to change the query template
   * @see localsolr
   *
   * @param string $qt
   * the query template name to use
   */
  public function change_query_template($qt) {
    $this->_query_template = $qt;
  }
  
  /**
   * add a field and value to the query string
   * This is useful for generating links with the facet blocks
   *
   * @param string $field
   * the query field
   *
   * @param string $value
   * the value of hte query field
   */
  function add_url_query_param($field, $value) {
    $this->_extra_query[$field] = $value;
  }
  
  public function get_available_sorts() {
    return $this->_available_sorts;
  }
  
  /**
   * Remove a specific subquery
   *
   * @param Drupal_Solr_Query_Interface $query
   * the query to remove
   */
  function remove_subquery(Drupal_Solr_Query_Interface $query) {
    unset($this->_subqueries[$query->id]);
  }
  
  /**
   * remove all subqueries
   */
  function remove_subqueries() {
    $this->_subqueries = array();    
  }
  
  /**
   * make a sort available
   */
  function set_available_sort($field, $sort) {
    // TODO: hmm.. wierd... we pull in all the sorts from Views data fields
  }
  
  /**
   * build up the filter query params that are passed to Solr
   */
  protected function rebuild_fq() {
    $query_filters = array();
    foreach ($this->_facets as $type => $facets) {
      $filter_string = '';
      foreach ($facets as $definition) {
        if ($definition['exclude']) {
          $type_string = "NOT $type";
        }
        else {
          $type_string = $type;
        }
        $query_filters[] = "$type_string:" . $definition['value'];
      }
    }
    
    return $query_filters;
  }
}