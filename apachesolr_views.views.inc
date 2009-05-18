<?php
// $Id$

/*
 * Load files with base classes of the contained classes.
 */

/**
 * Implementation of hook_views_handlers().
 */
function apachesolr_views_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'apachesolr_views') . '/handlers',
    ),
    'handlers' => array(
      'apachesolr_views_handler_argument' => array(
        'parent' => 'views_handler_argument',
      ),
      'apachesolr_views_handler_argument_mlt' => array(
        'parent' => 'views_handler_argument_node_nid',
      ),
      'apachesolr_views_handler_argument_tid' => array(
        'parent' => 'apachesolr_views_handler_argument',
      ),
      'apachesolr_views_handler_field' => array(
        'parent' => 'views_handler_field_node',
      ),
      'apachesolr_views_handler_field_author' => array(
        'parent' => 'apachesolr_views_handler_field',
      ),
      'apachesolr_views_handler_field_date' => array(
        'parent' => 'apachesolr_views_handler_field',
      ),
      'apachesolr_views_handler_field_generic' => array(
        'parent' => 'views_object',
      ),
      'apachesolr_views_handler_field_taxonomy' => array(
        'parent' => 'apachesolr_views_handler_field',
      ),
      'apachesolr_views_handler_field_type' => array(
        'parent' => 'apachesolr_views_handler_field',
      ),
      'apachesolr_views_handler_field_score' => array(
        'parent' => 'views_handler_field_numeric',
      ),
      'apachesolr_views_handler_field_snippet' => array(
        'parent' => 'views_handler_field',
      ),
      'apachesolr_views_handler_optionwidget_filter' => array(
        'parent' => 'views_handler_filter',
      ),
      'apachesolr_views_handler_filter_author' => array(
        'parent' => 'views_handler_filter_user_name',
      ),
      'apachesolr_views_handler_filter_search' => array(
        'parent' => 'views_handler_filter_search',
      ),
      'apachesolr_views_handler_filter_type' => array(
        'parent' => 'views_handler_filter_in_operator',
      ),
      'apachesolr_views_handler_sort' => array(
        'parent' => 'views_handler_sort',
      ),
    ),
  );
}

/**
 * Implementation of hook_views_plugins().
 */
function apachesolr_views_views_plugins() {
  return array(
    'module' => 'apachesolr_views',
    'query' => array(
      'apachesolr_views_query' => array(
        'title' => t('Apache Solr Query'),
        'help' => t('Query that allows you to search with Apache Solr.'),
        'handler' => 'apachesolr_views_query',
        'parent' => 'views_query',
      ),
    ),
  );

}

/**
 * Implementation of hook_views_data().
 */
function apachesolr_views_views_data() {
  $data['apachesolr']['table']['group']  = t('Apache Solr');

  $data['apachesolr']['table']['base'] = array(
    'query class' => 'apachesolr_views_query',
    'title' => t('Apache Solr'),
    'help' => t('Searches the site with the Apache Solr search engine.'),
    'field' => 'nid',
  );

  $data['apachesolr']['nid'] = array(
    'title' => t('Nid'),
    'help' => t('The node ID of the node.'),
    'field' => array(
      'name field' => 'title',
      'numeric' => TRUE,
      'handler' => 'apachesolr_views_handler_field_generic',
      'click sortable' => TRUE,
      'apachesolr base handler' => array(
        'table' => 'node',
        'field' => 'nid',
      ),
    ),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
  );
  $data['apachesolr']['title'] = array(
    'title' => t('Title'),
    'help' => t('The title of the node.'),
    'argument' => array(
      'handler' => 'apachesolr_views_handler_argument',
    ),
    'field' => array(
      'handler' => 'apachesolr_views_handler_field_generic',
      'click sortable' => TRUE,
      'apachesolr base handler' => array(
        'table' => 'node',
        'field' => 'title',
      ),
     ),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
  );
  $data['apachesolr']['created'] = array(
    'title' => t('Creation date'),
    'help' => t('The date the node was created.'),
    'field' => array(
      'handler' => 'apachesolr_views_handler_field_generic',
      'click sortable' => TRUE,
      'apachesolr base handler' => array(
        'table' => 'node',
        'field' => 'created',
      ),
    ),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
  );
  $data['apachesolr']['changed'] = array(
    'title' => t('Updated date'),
    'help' => t('The date the node was last updated.'),
    'field' => array(
      'handler' => 'apachesolr_views_handler_field_generic',
      'click sortable' => TRUE,
      'apachesolr base handler' => array(
        'table' => 'node',
        'field' => 'changed',
      ),
    ),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
  );
  $data['apachesolr']['type'] = array(
    'title' => t('Type'),
    'help' => t('The type of a node (for example, "blog entry", "forum post", "story", etc).'),
    'argument' => array(
      'handler' => 'apachesolr_views_handler_argument',
    ),
    'field' => array(
      'handler' => 'apachesolr_views_handler_field_generic',
      'click sortable' => TRUE,
      'apachesolr base handler' => array(
        'table' => 'node',
        'field' => 'type',
      ),
    ),
    'filter' => array(
      'handler' => 'apachesolr_views_handler_filter_type',
    ),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
  );
  $data['apachesolr']['name'] = array(
    'title' => t('Author'),
    'help' => t("The node's author."),
    'argument' => array(
      'handler' => 'apachesolr_views_handler_argument',
    ),
    'field' => array(
      'handler' => 'apachesolr_views_handler_field_generic',
      'click sortable' => TRUE,
      'additional fields' => array('uid'),
      'apachesolr base handler' => array(
        'table' => 'users',
        'field' => 'name',
      ),
    ),
    'filter' => array(
      'handler' => 'apachesolr_views_handler_filter_author',
    ),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
  );
  $data['apachesolr']['uid'] = array(
    'title' => t('Author Uid'),
    'help' => t("The node's author's user ID."),
    'argument' => array(
      'handler' => 'apachesolr_views_handler_argument',
    ),
    'field' => array(
      'name field' => 'name',
      'numeric' => TRUE,
      'handler' => 'apachesolr_views_handler_field_generic',
      'click sortable' => TRUE,
      'apachesolr base handler' => array(
        'table' => 'users',
        'field' => 'uid',
      ),
    ),
  );
  $data['apachesolr']['body'] = array(
    'title' => t('Body'),
    'help' => t("The node's content."),
    'field' => array(
      'handler' => 'apachesolr_views_handler_field_generic',
      'click sortable' => FALSE,
      'element type' => 'div',
      'apachesolr base handler' => array(
        'table' => 'node_revisions',
        'field' => 'body',
      ),
    ),
  );
  $data['apachesolr']['comment_count'] = array(
    'title' => t('Comment count'),
    'help' => t('The number of comments that were posted to the node.'),
    'field' => array(
      'numeric' => TRUE,
      'handler' => 'apachesolr_views_handler_field_generic',
      'click sortable' => TRUE,
      'apachesolr base handler' => array(
        'table' => 'node_comment_statistics',
        'field' => 'comment_count',
      ),
    ),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
  );
  // TODO Get taxonomy fields to work
  if (module_exists('taxonomy')) {
    $data['apachesolr']['tid'] = array(
      'title' => t('Taxonomy terms'),
      'help' => t('Taxonomy terms associated with the node.'),
      'argument' => array(
        'handler' => 'apachesolr_views_handler_argument_tid',
      ),
    );
  }
  if (module_exists('translation')) {
    $data['apachesolr']['language'] = array(
      'title' => t('Language'),
      'help' => t('The language the node is in.'),
      'argument' => array(
        'handler' => 'apachesolr_views_handler_argument',
      ),
      'field' => array(
        'handler' => 'apachesolr_views_handler_field_generic',
        'click sortable' => TRUE,
        'apachesolr base handler' => array(
          'table' => 'node',
          'field' => 'language',
        ),
      ),
      'sort' => array(
        'handler' => 'apachesolr_views_handler_sort',
      ),
    );
  }
  
  $data['apachesolr']['text'] = array(
    'title' => t('Search'),
    'help' => t('Searches the content with Solr'),
    'filter' => array(
      'handler' => 'apachesolr_views_handler_filter_search',
    ),
  );
  
  // score field. Useful for when combining sorts. So you sort by score, creation etc.
  $data['apachesolr']['score'] = array(
    'title' => t('Search Score'),
    'help' => t('The score of the search for this item. When no sorts are present this is the default sort'),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
    'field' => array(
      'handler' => 'apachesolr_views_handler_field_score'
    ),
  );
  
  // comment_count field useful for sorting
  $data['apachesolr']['comment_count'] = array(
    'title' => t('Comment Count'),
    'help' => t('The number of comments on a node'),
    'sort' => array(
      'handler' => 'apachesolr_views_handler_sort',
    ),
    'field' => array(
      'handler' => 'apachesolr_views_handler_field_generic',
      'click sortable' => FALSE,
      'element type' => 'div',
      'apachesolr base handler' => array(
        'table' => 'node_comment_statistics',
        'field' => 'comment_count',
      ),
    )
  );
  
  // snippet field
  $data['apachesolr']['snippet'] = array(
    'title' => t('Search Snippet'),
    'help' => t('The return snippet that matches the query sent to Solr.'),
    'field' => array(
      'handler' => 'apachesolr_views_handler_field_snippet',
    ),
  );
  $data['apachesolr']['mlt'] = array(
    'title' => t('More Like This Node id'),
    'help' => t('Do a more like this query based on the provided nid'),
    'argument' => array(
      'handler' => 'apachesolr_views_handler_argument_mlt',
    ),
  );
  
  // provide CCK mappings filters
  foreach (apachesolr_cck_fields() as $name => $field) {
    // TODO: can we make use of CCK's mapping already and just override
    // the query part???
    $options = content_allowed_values(content_fields($this->definition['cck_field']['field_name']));
    if (!empty($options)) {
      $data['apachesolr'][apachesolr_index_key($field)] = array(
        'title' => t($field['label']),
        'help' => t('CCK Mapping for @fieldname', array('@fieldname' => $field['field_name'])),
        'filter' => array(
          'handler' => 'apachesolr_views_handler_optionwidget_filter',
          'cck_field' => $field,
        ),
      );
    }
  }
  
  return $data;
}
